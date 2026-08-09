#ifndef SCHELLING_VECINDARIO_H
#define SCHELLING_VECINDARIO_H

#include "schelling/configuracion.h"
#include "schelling/modelo.h"

#include <stdbool.h>

typedef struct
{
    int deltaFila;
    int deltaColumna;
    double peso;
} DesplazamientoVecino;

typedef struct
{
    int radio;
    int cantidadDesplazamientos;
    DesplazamientoVecino *desplazamientos;
} Vecindario;

bool crearVecindario(Vecindario *vecindario, int radio, double sigma);
void liberarVecindario(Vecindario *vecindario);
bool calcularProporciones(const Modelo *modelo, const Vecindario *vecindario, int idCelda,
                          double proporciones[CANTIDAD_CLASES], bool *aislado);
bool calcularProporcionesExcluyendo(const Modelo *modelo, const Vecindario *vecindario, int idCelda,
                                    int idHogarExcluido, double proporciones[CANTIDAD_CLASES],
                                    bool *aislado);
bool cumpleTolerancias(ClaseSocioeconomica clase, const double proporciones[CANTIDAD_CLASES],
                       const Configuracion *configuracion);
bool evaluarSatisfaccion(const Modelo *modelo, const Vecindario *vecindario, int idCelda,
                         ClaseSocioeconomica clase, const Configuracion *configuracion,
                         bool *aislado);
bool evaluarSatisfaccionExcluyendo(const Modelo *modelo, const Vecindario *vecindario, int idCelda,
                                   int idHogarExcluido, ClaseSocioeconomica clase,
                                   const Configuracion *configuracion, bool *aislado);

#endif
