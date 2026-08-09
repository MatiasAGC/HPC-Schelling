#ifndef SCHELLING_HASH_H
#define SCHELLING_HASH_H

#include "schelling/modelo.h"
#include "schelling/configuracion.h"

#include <stdint.h>

uint64_t calcularHashModelo(const Modelo *modelo);
uint64_t calcularHashConfiguracionModelo(const Configuracion *configuracion);

#endif
