#ifndef SCHELLING_INDICE_VACANTES_H
#define SCHELLING_INDICE_VACANTES_H

#include "schelling/modelo.h"

#include <stdbool.h>

typedef struct
{
    int tamanoBloque;
    int bloquesAncho;
    int bloquesAlto;
    int cantidadBloques;
    int cantidadVacantes;
    int *inicios;
    int *idsVacantes;
} IndiceVacantes;

bool crearIndiceVacantes(IndiceVacantes *indice, const Modelo *modelo, int tamanoBloque);
bool crearIndiceVacantesFiltrado(IndiceVacantes *indice, const Modelo *modelo, int tamanoBloque,
                                 const unsigned char *incluir);
void liberarIndiceVacantes(IndiceVacantes *indice);
int obtenerIdBloque(const IndiceVacantes *indice, int filaBloque, int columnaBloque);

#endif
