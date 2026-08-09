#ifndef SCHELLING_SIMULACION_MPI_H
#define SCHELLING_SIMULACION_MPI_H

#include "schelling/simulacion.h"

#include <mpi.h>

typedef struct
{
    int trabajoMinimo;
    int trabajoMaximo;
    double trabajoPromedio;
    double desbalance;
    int solicitudesRemotas;
    uint64_t bytesComunicados;
    double tiempoPreparacion;
    double tiempoIndices;
    double tiempoBusqueda;
    double tiempoComunicacion;
    double tiempoConsolidacion;
} EstadisticasParalelas;

bool ejecutarIteracionMpi(Modelo *modelo, const Vecindario *vecindario,
                          const Configuracion *configuracion, uint64_t iteracion,
                          MPI_Comm comunicador, MetricasIteracion *metricas,
                          EstadisticasParalelas *estadisticas);

#endif
