#include "schelling/simulacion_mpi.h"

#include "schelling/economia.h"
#include "schelling/particion.h"
#include "schelling/registro.h"
#include "schelling/simulacion_fases.h"

#include <omp.h>
#include <stdlib.h>
#include <string.h>

static bool prepararEstadoDistribuido(Modelo *modelo, const Vecindario *vecindario,
                                      const Configuracion *configuracion, uint64_t iteracion,
                                      const Particion *particion, MPI_Comm comunicador,
                                      MetricasIteracion *metricas)
{
    int *satisfaccionLocal = calloc((size_t)modelo->cantidadHogares, sizeof(int));
    int *satisfaccionGlobal = calloc((size_t)modelo->cantidadHogares, sizeof(int));
    int *aislamientoLocal = calloc((size_t)modelo->cantidadHogares, sizeof(int));
    int *aislamientoGlobal = calloc((size_t)modelo->cantidadHogares, sizeof(int));
    double *preciosLocales = calloc((size_t)modelo->cantidadCeldas, sizeof(double));
    double *preciosGlobales = calloc((size_t)modelo->cantidadCeldas, sizeof(double));
    bool exito = satisfaccionLocal != NULL && satisfaccionGlobal != NULL &&
                 aislamientoLocal != NULL && aislamientoGlobal != NULL && preciosLocales != NULL &&
                 preciosGlobales != NULL;
    int memoriaLocal = exito ? 1 : 0;
    int memoriaGlobal;

    MPI_Allreduce(&memoriaLocal, &memoriaGlobal, 1, MPI_INT, MPI_MIN, comunicador);

    if (memoriaGlobal == 0)
    {
        registrarError("no se pudo reservar memoria para el estado distribuido");
        goto liberar;
    }

#pragma omp parallel for schedule(static)
    for (int idHogar = 0; idHogar < modelo->cantidadHogares; idHogar++)
    {
        Hogar *hogar = &modelo->hogares[idHogar];

        if (obtenerDuenoCelda(particion, hogar->idCelda) == particion->rank)
        {
            bool aislado;
            satisfaccionLocal[idHogar] = evaluarSatisfaccion(modelo, vecindario, hogar->idCelda,
                                                             hogar->clase, configuracion, &aislado)
                                             ? 1
                                             : 0;
            aislamientoLocal[idHogar] = aislado ? 1 : 0;
        }
    }

    MPI_Allreduce(satisfaccionLocal, satisfaccionGlobal, modelo->cantidadHogares, MPI_INT, MPI_SUM,
                  comunicador);
    MPI_Allreduce(aislamientoLocal, aislamientoGlobal, modelo->cantidadHogares, MPI_INT, MPI_SUM,
                  comunicador);
    memset(metricas, 0, sizeof(*metricas));

    for (int idHogar = 0; idHogar < modelo->cantidadHogares; idHogar++)
    {
        Hogar *hogar = &modelo->hogares[idHogar];
        hogar->satisfecho = satisfaccionGlobal[idHogar] != 0;

        if (hogar->satisfecho)
        {
            metricas->satisfechos++;
            metricas->aislados += aislamientoGlobal[idHogar];
        }
        else if (hogar->mesesBloqueado > 0)
        {
            metricas->insatisfechosBloqueados++;
        }
    }

    int preciosValidos = 1;

#pragma omp parallel for schedule(static) reduction(min : preciosValidos)
    for (int idCelda = 0; idCelda < modelo->cantidadCeldas; idCelda++)
    {
        Celda *celda = &modelo->celdas[idCelda];

        if (celda->tipo == CELDA_RESIDENCIAL && celda->idHogar == ID_INVALIDO &&
            obtenerDuenoCelda(particion, idCelda) == particion->rank)
        {
            bool precioValido =
                actualizarPrecioVacio(modelo, vecindario, configuracion, iteracion, idCelda);
            preciosLocales[idCelda] = celda->precio;
            preciosValidos = precioValido ? preciosValidos : 0;
        }
    }

    exito = preciosValidos != 0;

    MPI_Allreduce(preciosLocales, preciosGlobales, modelo->cantidadCeldas, MPI_DOUBLE, MPI_SUM,
                  comunicador);

    if (exito)
    {
        for (int idCelda = 0; idCelda < modelo->cantidadCeldas; idCelda++)
        {
            Celda *celda = &modelo->celdas[idCelda];

            if (celda->tipo == CELDA_RESIDENCIAL && celda->idHogar == ID_INVALIDO)
            {
                celda->precio = preciosGlobales[idCelda];
            }
        }
    }

liberar:
    free(satisfaccionLocal);
    free(satisfaccionGlobal);
    free(aislamientoLocal);
    free(aislamientoGlobal);
    free(preciosLocales);
    free(preciosGlobales);
    return exito;
}

static bool intercambiarHalos(Modelo *modelo, const Particion *particion, MPI_Comm comunicador)
{
    MPI_Request solicitudes[4];
    int cantidadSolicitudes = 0;
    int cantidadCeldas = particion->radioHalo * modelo->ancho;
    int cantidadBytes = cantidadCeldas * (int)sizeof(Celda);

    if (particion->radioHalo == 0)
    {
        return true;
    }

    if (particion->rank > 0)
    {
        Celda *haloSuperior =
            &modelo->celdas[(particion->primeraFila - particion->radioHalo) * modelo->ancho];
        Celda *bordeSuperior = &modelo->celdas[particion->primeraFila * modelo->ancho];
        MPI_Irecv(haloSuperior, cantidadBytes, MPI_BYTE, particion->rank - 1, 101, comunicador,
                  &solicitudes[cantidadSolicitudes++]);
        MPI_Isend(bordeSuperior, cantidadBytes, MPI_BYTE, particion->rank - 1, 102, comunicador,
                  &solicitudes[cantidadSolicitudes++]);
    }

    if (particion->rank + 1 < particion->procesos)
    {
        Celda *haloInferior = &modelo->celdas[particion->ultimaFila * modelo->ancho];
        Celda *bordeInferior =
            &modelo->celdas[(particion->ultimaFila - particion->radioHalo) * modelo->ancho];
        MPI_Irecv(haloInferior, cantidadBytes, MPI_BYTE, particion->rank + 1, 102, comunicador,
                  &solicitudes[cantidadSolicitudes++]);
        MPI_Isend(bordeInferior, cantidadBytes, MPI_BYTE, particion->rank + 1, 101, comunicador,
                  &solicitudes[cantidadSolicitudes++]);
    }

    return MPI_Waitall(cantidadSolicitudes, solicitudes, MPI_STATUSES_IGNORE) == MPI_SUCCESS;
}

bool ejecutarIteracionMpi(Modelo *modelo, const Vecindario *vecindario,
                          const Configuracion *configuracion, uint64_t iteracion,
                          MPI_Comm comunicador, MetricasIteracion *metricas, int *migraciones)
{
    IndicesVacantes indices = {0};
    Particion particion;
    int *destinosLocales;
    int *destinosGlobales;
    int rank;
    int procesos;
    int exitoLocal = 1;
    int exitoGlobal;

    if (modelo == NULL || vecindario == NULL || configuracion == NULL || metricas == NULL ||
        migraciones == NULL)
    {
        return false;
    }

    MPI_Comm_rank(comunicador, &rank);
    MPI_Comm_size(comunicador, &procesos);

    if (!crearParticion(&particion, rank, procesos, modelo->ancho, modelo->alto, vecindario->radio))
    {
        return false;
    }

    destinosLocales = malloc((size_t)modelo->cantidadHogares * sizeof(int));
    destinosGlobales = malloc((size_t)modelo->cantidadHogares * sizeof(int));

    if ((modelo->cantidadHogares > 0 && destinosLocales == NULL) ||
        (modelo->cantidadHogares > 0 && destinosGlobales == NULL))
    {
        free(destinosLocales);
        free(destinosGlobales);
        registrarError("no se pudo reservar memoria para las solicitudes mpi");
        return false;
    }

    if (!prepararEstadoDistribuido(modelo, vecindario, configuracion, iteracion, &particion,
                                   comunicador, metricas) ||
        !crearIndicesVacantes(&indices, modelo, vecindario, configuracion))
    {
        exitoLocal = 0;
    }

    MPI_Allreduce(&exitoLocal, &exitoGlobal, 1, MPI_INT, MPI_MIN, comunicador);

    if (exitoGlobal == 0)
    {
        liberarIndicesVacantes(&indices);
        free(destinosLocales);
        free(destinosGlobales);
        return false;
    }

#pragma omp parallel for schedule(static)
    for (int idHogar = 0; idHogar < modelo->cantidadHogares; idHogar++)
    {
        const Hogar *hogar = &modelo->hogares[idHogar];
        bool esLocal = obtenerDuenoCelda(&particion, hogar->idCelda) == rank;

        destinosLocales[idHogar] =
            esLocal && !hogar->satisfecho && hogar->mesesBloqueado == 0
                ? buscarDestinoHogar(modelo, vecindario, configuracion, &indices, idHogar)
                : ID_INVALIDO;
    }

    MPI_Allreduce(destinosLocales, destinosGlobales, modelo->cantidadHogares, MPI_INT, MPI_MAX,
                  comunicador);

    *migraciones = 0;
    for (int idHogar = 0; idHogar < modelo->cantidadHogares; idHogar++)
    {
        if (destinosGlobales[idHogar] != ID_INVALIDO &&
            obtenerDuenoCelda(&particion, modelo->hogares[idHogar].idCelda) !=
                obtenerDuenoCelda(&particion, destinosGlobales[idHogar]))
        {
            (*migraciones)++;
        }
    }

    bool resultado =
        consolidarMudanzas(modelo, configuracion, iteracion, destinosGlobales, metricas) &&
        intercambiarHalos(modelo, &particion, comunicador);
    liberarIndicesVacantes(&indices);
    free(destinosLocales);
    free(destinosGlobales);
    return resultado;
}
