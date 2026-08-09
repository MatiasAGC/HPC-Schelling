#include "schelling/simulacion.h"

#include "schelling/aleatorio.h"
#include "schelling/economia.h"
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

static int buscarDestino(const Modelo *modelo, const Vecindario *vecindario,
                         const Configuracion *configuracion, int idHogar)
{
    const Hogar *hogar = &modelo->hogares[idHogar];
    int mejorDestino = ID_INVALIDO;
    long long mejorDistancia = 0;

    for (int idCelda = 0; idCelda < modelo->cantidadCeldas; idCelda++)
    {
        const Celda *celda = &modelo->celdas[idCelda];
        bool aislado;

        if (celda->tipo != CELDA_RESIDENCIAL || celda->idHogar != ID_INVALIDO ||
            !esViviendaAccesible(celda->precio, hogar, configuracion) ||
            !evaluarSatisfaccionExcluyendo(modelo, vecindario, idCelda, idHogar, hogar->clase,
                                           configuracion, &aislado))
        {
            continue;
        }

        long long distancia = distanciaCuadrada(modelo, hogar->idCelda, idCelda);

        if (mejorDestino == ID_INVALIDO || distancia < mejorDistancia ||
            (distancia == mejorDistancia && idCelda < mejorDestino))
        {
            mejorDestino = idCelda;
            mejorDistancia = distancia;
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

    for (int idHogar = 0; idHogar < modelo->cantidadHogares; idHogar++)
    {
        Hogar *hogar = &modelo->hogares[idHogar];

        if (!hogar->satisfecho && hogar->mesesBloqueado == 0)
        {
            int destino = buscarDestino(modelo, vecindario, configuracion, idHogar);
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
    return validarModelo(modelo);
}
