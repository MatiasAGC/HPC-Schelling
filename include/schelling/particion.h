#ifndef SCHELLING_PARTICION_H
#define SCHELLING_PARTICION_H

#include <stdbool.h>

typedef struct
{
    int rank;
    int procesos;
    int ancho;
    int alto;
    int radioHalo;
    int primeraFila;
    int ultimaFila;
    int primeraFilaConHalo;
    int ultimaFilaConHalo;
} Particion;

bool crearParticion(Particion *particion, int rank, int procesos, int ancho, int alto,
                    int radioHalo);
int obtenerDuenoFila(const Particion *particion, int fila);
int obtenerDuenoCelda(const Particion *particion, int idCelda);
bool esFilaLocal(const Particion *particion, int fila);

#endif
