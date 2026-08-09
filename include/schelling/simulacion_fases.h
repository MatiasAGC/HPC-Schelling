#ifndef SCHELLING_SIMULACION_FASES_H
#define SCHELLING_SIMULACION_FASES_H

#include "schelling/indice_vacantes.h"
#include "schelling/simulacion.h"

typedef struct
{
    IndiceVacantes porSubestrato[CANTIDAD_SUBESTRATOS];
} IndicesVacantes;

bool evaluarEstado(Modelo *modelo, const Vecindario *vecindario, const Configuracion *configuracion,
                   MetricasIteracion *metricas);
bool crearIndicesVacantes(IndicesVacantes *indices, const Modelo *modelo,
                          const Vecindario *vecindario, const Configuracion *configuracion);
void liberarIndicesVacantes(IndicesVacantes *indices);
int buscarDestinoHogar(const Modelo *modelo, const Vecindario *vecindario,
                       const Configuracion *configuracion, const IndicesVacantes *indices,
                       int idHogar);
bool consolidarMudanzas(Modelo *modelo, const Configuracion *configuracion, uint64_t iteracion,
                        const int *destinos, MetricasIteracion *metricas);

#endif
