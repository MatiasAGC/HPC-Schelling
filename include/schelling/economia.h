#ifndef SCHELLING_ECONOMIA_H
#define SCHELLING_ECONOMIA_H

#include "schelling/configuracion.h"
#include "schelling/modelo.h"
#include "schelling/vecindario.h"

#include <stdbool.h>
#include <stdint.h>

double calcularCuotaMensual(double precio, const Configuracion *configuracion);
bool esViviendaAccesible(double precio, const Hogar *hogar, const Configuracion *configuracion);
bool calcularEntornoEconomico(const Modelo *modelo, const Vecindario *vecindario, int idCelda,
                              double *demanda, double *poderAdquisitivo);
bool actualizarPreciosVacios(Modelo *modelo, const Vecindario *vecindario,
                             const Configuracion *configuracion, uint64_t iteracion);
bool actualizarPrecioVacio(Modelo *modelo, const Vecindario *vecindario,
                           const Configuracion *configuracion, uint64_t iteracion, int idCelda);

#endif
