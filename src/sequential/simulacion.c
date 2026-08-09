#include "schelling/simulacion.h"

#include "schelling/aleatorio.h"
#include "schelling/economia.h"
#include "schelling/indice_vacantes.h"
#include "schelling/registro.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

static long long distanciaCuadrada(const Modelo *modelo, int origen, int destino)
{
    int filaOrigen;
    int columnaOrigen;
    int filaDestino;
    int columnaDestino;

    obtenerCoordenadas(modelo, origen, &filaOrigen, &columnaOrigen);
    obtenerCoordenadas(modelo, destino, &filaDestino, &columnaDestino);

    long long diferenciaFila = (long long)filaOrigen - filaDestino;
    long long diferenciaColumna = (long long)columnaOrigen - columnaDestino;
    return diferenciaFila * diferenciaFila + diferenciaColumna * diferenciaColumna;
}

static void evaluarCandidato(const Modelo *modelo, const Vecindario *vecindario,
                             const Configuracion *configuracion, int idHogar, int idCelda,
                             bool satisfaccionPrecalculada, int *mejorDestino,
                             long long *mejorDistancia)
{
    const Hogar *hogar = &modelo->hogares[idHogar];
    const Celda *celda = &modelo->celdas[idCelda];
    bool aislado;
    int filaOrigen;
    int columnaOrigen;
    int filaDestino;
    int columnaDestino;
    bool satisface = satisfaccionPrecalculada;

    obtenerCoordenadas(modelo, hogar->idCelda, &filaOrigen, &columnaOrigen);
    obtenerCoordenadas(modelo, idCelda, &filaDestino, &columnaDestino);

    if (abs(filaOrigen - filaDestino) <= vecindario->radio &&
        abs(columnaOrigen - columnaDestino) <= vecindario->radio)
    {
        satisface = evaluarSatisfaccionExcluyendo(modelo, vecindario, idCelda, idHogar,
                                                  hogar->clase, configuracion, &aislado);
    }

    if (!esViviendaAccesible(celda->precio, hogar, configuracion) || !satisface)
    {
        return;
    }

    long long distancia = distanciaCuadrada(modelo, hogar->idCelda, idCelda);

    if (*mejorDestino == ID_INVALIDO || distancia < *mejorDistancia ||
        (distancia == *mejorDistancia && idCelda < *mejorDestino))
    {
        *mejorDestino = idCelda;
        *mejorDistancia = distancia;
    }
}

static int buscarDestino(const Modelo *modelo, const Vecindario *vecindario,
                         const Configuracion *configuracion, const IndiceVacantes *indice,
                         int idHogar)
{
    const Hogar *hogar = &modelo->hogares[idHogar];
    int filaOrigen;
    int columnaOrigen;
    int filaBloqueOrigen;
    int columnaBloqueOrigen;
    int mejorDestino = ID_INVALIDO;
    long long mejorDistancia = 0;
    int radioMaximo =
        indice->bloquesAncho > indice->bloquesAlto ? indice->bloquesAncho : indice->bloquesAlto;

    obtenerCoordenadas(modelo, hogar->idCelda, &filaOrigen, &columnaOrigen);
    filaBloqueOrigen = filaOrigen / indice->tamanoBloque;
    columnaBloqueOrigen = columnaOrigen / indice->tamanoBloque;

    for (int radio = 0; radio < radioMaximo; radio++)
    {
        for (int deltaFila = -radio; deltaFila <= radio; deltaFila++)
        {
            for (int deltaColumna = -radio; deltaColumna <= radio; deltaColumna++)
            {
                if (radio > 0 && abs(deltaFila) != radio && abs(deltaColumna) != radio)
                {
                    continue;
                }

                int idBloque = obtenerIdBloque(indice, filaBloqueOrigen + deltaFila,
                                               columnaBloqueOrigen + deltaColumna);

                if (idBloque == ID_INVALIDO)
                {
                    continue;
                }

                for (int posicion = indice->inicios[idBloque];
                     posicion < indice->inicios[idBloque + 1]; posicion++)
                {
                    evaluarCandidato(modelo, vecindario, configuracion, idHogar,
                                     indice->idsVacantes[posicion], true, &mejorDestino,
                                     &mejorDistancia);
                }
            }
        }

        long long distanciaExterior = (long long)radio * indice->tamanoBloque + 1;

        if (mejorDestino != ID_INVALIDO && distanciaExterior * distanciaExterior > mejorDistancia)
        {
            break;
        }
    }

    return mejorDestino;
}

static bool tienePrioridad(const Modelo *modelo, const Configuracion *configuracion,
                           uint64_t iteracion, int candidato, int actual)
{
    const Hogar *hogarCandidato = &modelo->hogares[candidato];
    const Hogar *hogarActual = &modelo->hogares[actual];

    if (hogarCandidato->ingresoMensual != hogarActual->ingresoMensual)
    {
        return hogarCandidato->ingresoMensual > hogarActual->ingresoMensual;
    }

    uint64_t claveCandidato = generarBitsAleatorios(configuracion->semilla, iteracion,
                                                    (uint64_t)candidato, PROPOSITO_DESEMPATE, 0);
    uint64_t claveActual = generarBitsAleatorios(configuracion->semilla, iteracion,
                                                 (uint64_t)actual, PROPOSITO_DESEMPATE, 0);

    return claveCandidato < claveActual || (claveCandidato == claveActual && candidato < actual);
}

bool ejecutarIteracion(Modelo *modelo, const Vecindario *vecindario,
                       const Configuracion *configuracion, uint64_t iteracion,
                       MetricasIteracion *metricas)
{
    int *destinos;
    int *ganadores;
    unsigned char *satisfacciones;
    unsigned char *incluir;
    IndiceVacantes indices[CANTIDAD_SUBESTRATOS] = {0};

    if (modelo == NULL || vecindario == NULL || configuracion == NULL || metricas == NULL)
    {
        return false;
    }

    destinos = malloc((size_t)modelo->cantidadHogares * sizeof(int));
    ganadores = malloc((size_t)modelo->cantidadCeldas * sizeof(int));

    if ((modelo->cantidadHogares > 0 && destinos == NULL) || ganadores == NULL)
    {
        free(destinos);
        free(ganadores);
        registrarError("no se pudo reservar memoria para una iteracion");
        return false;
    }

    memset(metricas, 0, sizeof(*metricas));

    for (int idCelda = 0; idCelda < modelo->cantidadCeldas; idCelda++)
    {
        ganadores[idCelda] = ID_INVALIDO;
    }

    for (int idHogar = 0; idHogar < modelo->cantidadHogares; idHogar++)
    {
        Hogar *hogar = &modelo->hogares[idHogar];
        bool aislado;

        hogar->satisfecho = evaluarSatisfaccion(modelo, vecindario, hogar->idCelda, hogar->clase,
                                                configuracion, &aislado);
        destinos[idHogar] = ID_INVALIDO;

        if (hogar->satisfecho)
        {
            metricas->satisfechos++;
            metricas->aislados += aislado ? 1 : 0;
        }
        else if (hogar->mesesBloqueado > 0)
        {
            metricas->insatisfechosBloqueados++;
        }
    }

    if (!actualizarPreciosVacios(modelo, vecindario, configuracion, iteracion))
    {
        free(destinos);
        free(ganadores);
        return false;
    }

    satisfacciones = calloc((size_t)modelo->cantidadCeldas * CANTIDAD_CLASES, 1);
    incluir = calloc((size_t)modelo->cantidadCeldas, 1);

    if (satisfacciones == NULL || incluir == NULL)
    {
        free(satisfacciones);
        free(incluir);
        free(destinos);
        free(ganadores);
        registrarError("no se pudo reservar memoria para filtrar las vacantes");
        return false;
    }

    for (int idCelda = 0; idCelda < modelo->cantidadCeldas; idCelda++)
    {
        const Celda *celda = &modelo->celdas[idCelda];

        if (celda->tipo == CELDA_RESIDENCIAL && celda->idHogar == ID_INVALIDO)
        {
            for (int clase = 0; clase < CANTIDAD_CLASES; clase++)
            {
                bool aislado;
                size_t posicion = (size_t)idCelda * CANTIDAD_CLASES + (size_t)clase;
                satisfacciones[posicion] =
                    evaluarSatisfaccion(modelo, vecindario, idCelda, (ClaseSocioeconomica)clase,
                                        configuracion, &aislado)
                        ? 1
                        : 0;
            }
        }
    }

    for (int subestrato = 0; subestrato < CANTIDAD_SUBESTRATOS; subestrato++)
    {
        Hogar hogarReferencia = {0};
        hogarReferencia.subestrato = (Subestrato)subestrato;
        hogarReferencia.clase = obtenerClaseSubestrato(hogarReferencia.subestrato);
        hogarReferencia.ingresoMensual = obtenerIngresoBase(hogarReferencia.subestrato);

        for (int idCelda = 0; idCelda < modelo->cantidadCeldas; idCelda++)
        {
            size_t posicion =
                (size_t)idCelda * CANTIDAD_CLASES + (size_t)(int)hogarReferencia.clase;
            incluir[idCelda] = satisfacciones[posicion] != 0 &&
                                       esViviendaAccesible(modelo->celdas[idCelda].precio,
                                                           &hogarReferencia, configuracion)
                                   ? 1
                                   : 0;
        }

        if (!crearIndiceVacantesFiltrado(&indices[subestrato], modelo,
                                         configuracion->tamanoBloqueVacantes, incluir))
        {
            for (int indice = 0; indice < subestrato; indice++)
            {
                liberarIndiceVacantes(&indices[indice]);
            }
            free(satisfacciones);
            free(incluir);
            free(destinos);
            free(ganadores);
            return false;
        }
    }

    free(satisfacciones);
    free(incluir);

    for (int idHogar = 0; idHogar < modelo->cantidadHogares; idHogar++)
    {
        Hogar *hogar = &modelo->hogares[idHogar];

        if (!hogar->satisfecho && hogar->mesesBloqueado == 0)
        {
            int destino = buscarDestino(modelo, vecindario, configuracion,
                                        &indices[(int)hogar->subestrato], idHogar);
            destinos[idHogar] = destino;

            if (destino == ID_INVALIDO)
            {
                metricas->sinDestino++;
            }
            else
            {
                metricas->solicitudes++;

                if (ganadores[destino] == ID_INVALIDO ||
                    tienePrioridad(modelo, configuracion, iteracion, idHogar, ganadores[destino]))
                {
                    ganadores[destino] = idHogar;
                }
            }
        }
    }

    for (int idCelda = 0; idCelda < modelo->cantidadCeldas; idCelda++)
    {
        int ganador = ganadores[idCelda];

        if (ganador != ID_INVALIDO)
        {
            modelo->celdas[modelo->hogares[ganador].idCelda].idHogar = ID_INVALIDO;
        }
    }

    for (int idCelda = 0; idCelda < modelo->cantidadCeldas; idCelda++)
    {
        int ganador = ganadores[idCelda];

        if (ganador != ID_INVALIDO)
        {
            modelo->celdas[idCelda].idHogar = ganador;
            modelo->hogares[ganador].idCelda = idCelda;
            modelo->hogares[ganador].mesesBloqueado = configuracion->permanenciaMinima;
            metricas->aceptadas++;
        }
    }

    metricas->rechazadas = metricas->solicitudes - metricas->aceptadas;

    for (int idHogar = 0; idHogar < modelo->cantidadHogares; idHogar++)
    {
        bool seMudo = destinos[idHogar] != ID_INVALIDO && ganadores[destinos[idHogar]] == idHogar;

        if (!seMudo && modelo->hogares[idHogar].mesesBloqueado > 0)
        {
            modelo->hogares[idHogar].mesesBloqueado--;
        }
    }

    free(destinos);
    free(ganadores);
    for (int indice = 0; indice < CANTIDAD_SUBESTRATOS; indice++)
    {
        liberarIndiceVacantes(&indices[indice]);
    }
    return validarModelo(modelo);
}
