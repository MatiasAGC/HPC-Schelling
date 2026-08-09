#ifndef SCHELLING_CHECKPOINT_H
#define SCHELLING_CHECKPOINT_H

#include "schelling/modelo.h"
#include "schelling/configuracion.h"

#include <stdbool.h>
#include <stdint.h>

bool guardarCheckpoint(const char *ruta, const Modelo *modelo, const Configuracion *configuracion,
                       uint64_t proximaIteracion);
bool cargarCheckpoint(const char *ruta, Modelo *modelo, const Configuracion *configuracion,
                      uint64_t *proximaIteracion);

#endif
