#ifndef SCHELLING_SIMULACION_H
#define SCHELLING_SIMULACION_H

#include "schelling/configuracion.h"
#include "schelling/modelo.h"
#include "schelling/vecindario.h"

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    int satisfechos;
    int aislados;
    int insatisfechosBloqueados;
    int solicitudes;
    int aceptadas;
    int rechazadas;
    int sinDestino;
} MetricasIteracion;

bool ejecutarIteracion(Modelo *modelo, const Vecindario *vecindario,
                       const Configuracion *configuracion, uint64_t iteracion,
                       MetricasIteracion *metricas);

#endif
