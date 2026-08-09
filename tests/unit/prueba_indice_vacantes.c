#include "schelling/indice_vacantes.h"

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

static void indiceAgrupaVacantesFiltradas(void)
{
    Modelo modelo = {0};
    IndiceVacantes indice = {0};
    unsigned char incluir[12] = {0};

    verificar(crearModelo(&modelo, 4, 3, 1), "no se pudo crear el modelo");
    verificar(definirCelda(&modelo, 5, CELDA_NO_RESIDENCIAL, 0, 0.0),
              "no se pudo definir la celda no residencial");
    verificar(ubicarHogar(&modelo, 0, 1, SUBESTRATO_M, obtenerIngresoBase(SUBESTRATO_M)),
              "no se pudo ubicar el hogar");

    incluir[0] = 1;
    incluir[1] = 1;
    incluir[3] = 1;
    incluir[5] = 1;
    incluir[8] = 1;
    incluir[11] = 1;

    verificar(crearIndiceVacantesFiltrado(&indice, &modelo, 2, incluir),
              "no se pudo crear el indice filtrado");
    verificar(indice.bloquesAncho == 2 && indice.bloquesAlto == 2,
              "la division en bloques es incorrecta");
    verificar(indice.cantidadVacantes == 4, "la cantidad de vacantes filtradas es incorrecta");
    verificar(indice.inicios[0] == 0 && indice.inicios[1] == 1 && indice.inicios[2] == 2 &&
                  indice.inicios[3] == 3 && indice.inicios[4] == 4,
              "los limites de los bloques son incorrectos");
    verificar(indice.idsVacantes[0] == 0 && indice.idsVacantes[1] == 3 &&
                  indice.idsVacantes[2] == 8 && indice.idsVacantes[3] == 11,
              "el orden de las vacantes es incorrecto");
    verificar(obtenerIdBloque(&indice, 1, 1) == 3, "el identificador de bloque es incorrecto");
    verificar(obtenerIdBloque(&indice, 2, 0) == ID_INVALIDO,
              "un bloque exterior deberia ser invalido");

    liberarIndiceVacantes(&indice);
    liberarModelo(&modelo);
}

int main(void)
{
    indiceAgrupaVacantesFiltradas();

    if (cantidadFallos != 0)
    {
        fprintf(stderr, "fallaron %d verificaciones\n", cantidadFallos);
        return 1;
    }

    printf("pruebas del indice de vacantes correctas\n");
    return 0;
}
