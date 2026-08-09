#include "schelling/particion.h"

#include <stdio.h>

static int cantidadFallos = 0;

static void verificar(bool condicion, const char *mensaje)
{
    if (!condicion)
    {
        fprintf(stderr, "%s\n", mensaje);
        cantidadFallos++;
    }
}

static void franjasCubrenLaGrilla(void)
{
    Particion particiones[3];

    for (int rank = 0; rank < 3; rank++)
    {
        verificar(crearParticion(&particiones[rank], rank, 3, 8, 7, 2),
                  "no se pudo crear una particion");
    }

    verificar(particiones[0].primeraFila == 0 && particiones[0].ultimaFila == 2,
              "la primera franja es incorrecta");
    verificar(particiones[1].primeraFila == 2 && particiones[1].ultimaFila == 4,
              "la segunda franja es incorrecta");
    verificar(particiones[2].primeraFila == 4 && particiones[2].ultimaFila == 7,
              "la tercera franja es incorrecta");
    verificar(particiones[0].primeraFilaConHalo == 0 && particiones[0].ultimaFilaConHalo == 4,
              "el halo superior es incorrecto");
    verificar(particiones[2].primeraFilaConHalo == 2 && particiones[2].ultimaFilaConHalo == 7,
              "el halo inferior es incorrecto");

    for (int fila = 0; fila < 7; fila++)
    {
        int dueno = obtenerDuenoFila(&particiones[0], fila);
        verificar(dueno >= 0 && dueno < 3 && esFilaLocal(&particiones[dueno], fila),
                  "una fila no tiene un unico dueno valido");
    }
}

int main(void)
{
    franjasCubrenLaGrilla();
    verificar(!crearParticion(NULL, 0, 1, 1, 1, 0), "una particion nula deberia fallar");

    if (cantidadFallos != 0)
    {
        fprintf(stderr, "fallaron %d verificaciones\n", cantidadFallos);
        return 1;
    }

    printf("pruebas de particion correctas\n");
    return 0;
}
