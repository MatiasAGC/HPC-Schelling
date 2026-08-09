#include "schelling/particion.h"

#include <string.h>

static int obtenerInicio(int indice, int cantidad, int total)
{
    return (int)(((long long)indice * total) / cantidad);
}

bool crearParticion(Particion *particion, int rank, int procesos, int ancho, int alto,
                    int radioHalo)
{
    if (particion == NULL || procesos <= 0 || rank < 0 || rank >= procesos || ancho <= 0 ||
        alto <= 0 || radioHalo < 0 || procesos > alto)
    {
        return false;
    }

    memset(particion, 0, sizeof(*particion));
    particion->rank = rank;
    particion->procesos = procesos;
    particion->ancho = ancho;
    particion->alto = alto;
    particion->radioHalo = radioHalo;
    particion->primeraFila = obtenerInicio(rank, procesos, alto);
    particion->ultimaFila = obtenerInicio(rank + 1, procesos, alto);
    particion->primeraFilaConHalo = particion->primeraFila - radioHalo;
    particion->ultimaFilaConHalo = particion->ultimaFila + radioHalo;

    if (particion->primeraFilaConHalo < 0)
    {
        particion->primeraFilaConHalo = 0;
    }
    if (particion->ultimaFilaConHalo > alto)
    {
        particion->ultimaFilaConHalo = alto;
    }

    return true;
}

int obtenerDuenoFila(const Particion *particion, int fila)
{
    if (particion == NULL || fila < 0 || fila >= particion->alto)
    {
        return -1;
    }

    int dueno = (int)(((long long)(fila + 1) * particion->procesos - 1) / particion->alto);
    return dueno < particion->procesos ? dueno : particion->procesos - 1;
}

int obtenerDuenoCelda(const Particion *particion, int idCelda)
{
    if (particion == NULL || idCelda < 0 || idCelda >= particion->ancho * particion->alto)
    {
        return -1;
    }

    return obtenerDuenoFila(particion, idCelda / particion->ancho);
}

bool esFilaLocal(const Particion *particion, int fila)
{
    return particion != NULL && fila >= particion->primeraFila && fila < particion->ultimaFila;
}
