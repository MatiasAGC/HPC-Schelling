#ifndef SCHELLING_ANALISIS_H
#define SCHELLING_ANALISIS_H

#include "schelling/modelo.h"
#include "schelling/vecindario.h"

#include <stdbool.h>

typedef struct
{
    double proporcionMismaClase;
    int hogaresConVecinos;
    int hogaresAislados;
} MetricasSegregacion;

bool calcularMetricasSegregacion(const Modelo *modelo, const Vecindario *vecindario,
                                 MetricasSegregacion *metricas);
bool escribirGrillaPpm(const char *ruta, const Modelo *modelo);

#endif
