#ifndef SCHELLING_SALIDA_H
#define SCHELLING_SALIDA_H

#include "schelling/configuracion.h"
#include "schelling/simulacion.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef struct
{
    char directorio[512];
    FILE *metricas;
    FILE *tiempos;
} SalidaEjecucion;

bool iniciarSalida(SalidaEjecucion *salida, const char *directorio,
                   const Configuracion *configuracion, const char *version, int procesos,
                   int threads);
bool escribirMetricas(SalidaEjecucion *salida, uint64_t iteracion,
                      const MetricasIteracion *metricas, uint64_t hash);
bool escribirTiempo(SalidaEjecucion *salida, uint64_t iteracion, double segundos);
bool crearRutaSalida(const SalidaEjecucion *salida, const char *nombre, char *ruta,
                     size_t longitudRuta);
void cerrarSalida(SalidaEjecucion *salida);

#endif
