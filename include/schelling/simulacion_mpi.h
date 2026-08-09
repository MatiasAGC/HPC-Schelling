#ifndef SCHELLING_SIMULACION_MPI_H
#define SCHELLING_SIMULACION_MPI_H

#include "schelling/simulacion.h"

#include <mpi.h>

bool ejecutarIteracionMpi(Modelo *modelo, const Vecindario *vecindario,
                          const Configuracion *configuracion, uint64_t iteracion,
                          MPI_Comm comunicador, MetricasIteracion *metricas, int *migraciones);

#endif
