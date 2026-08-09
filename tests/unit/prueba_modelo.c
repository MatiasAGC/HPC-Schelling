#include "schelling/modelo.h"

#include <math.h>
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

static void conversionesSonConsistentes(void)
{
    Modelo modelo = {0};
    int fila = ID_INVALIDO;
    int columna = ID_INVALIDO;

    verificar(crearModelo(&modelo, 3, 2, 0), "no se pudo crear el modelo");
    verificar(obtenerIdCelda(&modelo, 1, 2) == 5, "el id de celda es incorrecto");
    verificar(obtenerCoordenadas(&modelo, 4, &fila, &columna),
              "no se pudieron obtener las coordenadas");
    verificar(fila == 1 && columna == 1, "las coordenadas son incorrectas");
    verificar(obtenerIdCelda(&modelo, 2, 0) == ID_INVALIDO,
              "una fila exterior deberia ser invalida");
    verificar(!obtenerCoordenadas(&modelo, 6, &fila, &columna),
              "un id exterior deberia ser invalido");
    liberarModelo(&modelo);
}

static void ocupacionMantieneInvariantes(void)
{
    Modelo modelo = {0};

    verificar(crearModelo(&modelo, 3, 2, 2), "no se pudo crear el modelo con hogares");
    verificar(definirCelda(&modelo, 5, CELDA_NO_RESIDENCIAL, 1, 0.0),
              "no se pudo definir la celda no residencial");
    verificar(
        ubicarHogar(&modelo, 0, 0, SUBESTRATO_A_MENOS, obtenerIngresoBase(SUBESTRATO_A_MENOS)),
        "no se pudo ubicar el primer hogar");
    verificar(ubicarHogar(&modelo, 1, 4, SUBESTRATO_B_MAS, obtenerIngresoBase(SUBESTRATO_B_MAS)),
              "no se pudo ubicar el segundo hogar");
    verificar(modelo.hogares[0].clase == CLASE_ALTA, "la clase alta es incorrecta");
    verificar(modelo.hogares[1].clase == CLASE_BAJA, "la clase baja es incorrecta");
    verificar(validarModelo(&modelo), "el modelo completo deberia ser valido");
    verificar(!ubicarHogar(&modelo, 1, 1, SUBESTRATO_B_MAS, 16.1),
              "un hogar no puede ocupar dos viviendas");
    verificar(!ubicarHogar(&modelo, 0, 4, SUBESTRATO_A_MENOS, 71.0),
              "una vivienda no puede contener dos hogares");
    verificar(!definirCelda(&modelo, 0, CELDA_NO_RESIDENCIAL, 0, 0.0),
              "una vivienda ocupada no puede pasar a no residencial");
    verificar(!ubicarHogar(&modelo, 0, 5, SUBESTRATO_A_MENOS, 71.0),
              "una celda no residencial no puede ocuparse");
    liberarModelo(&modelo);
}

static void modeloIncompletoEsInvalido(void)
{
    Modelo modelo = {0};

    verificar(crearModelo(&modelo, 2, 2, 1), "no se pudo crear el modelo incompleto");
    verificar(!validarModelo(&modelo), "un hogar sin vivienda deberia invalidar el modelo");
    liberarModelo(&modelo);
}

int main(void)
{
    conversionesSonConsistentes();
    ocupacionMantieneInvariantes();
    modeloIncompletoEsInvalido();

    if (cantidadFallos != 0)
    {
        fprintf(stderr, "fallaron %d verificaciones\n", cantidadFallos);
        return 1;
    }

    printf("pruebas del modelo correctas\n");
    return 0;
}
