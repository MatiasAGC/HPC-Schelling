#include "schelling/configuracion.h"

#include <math.h>
#include <stdio.h>

static int cantidadFallos = 0;

static void verificarEntero(const char *nombre, int esperado, int obtenido)
{
    if (esperado != obtenido)
    {
        fprintf(stderr, "%s esperado %d obtenido %d\n", nombre, esperado, obtenido);
        cantidadFallos++;
    }
}

static void verificarReal(const char *nombre, double esperado, double obtenido)
{
    if (fabs(esperado - obtenido) > 1e-12)
    {
        fprintf(stderr, "%s esperado %.12f obtenido %.12f\n", nombre, esperado, obtenido);
        cantidadFallos++;
    }
}

static void valoresPredeterminadosSonValidos(void)
{
    Configuracion configuracion;

    iniciarConfiguracionPredeterminada(&configuracion);

    verificarEntero("ancho predeterminado", 1024, configuracion.ancho);
    verificarEntero("alto predeterminado", 640, configuracion.alto);
    verificarEntero("radio predeterminado", 2, configuracion.radioVecindario);
    verificarReal("sigma predeterminado", 1.0, configuracion.sigma);
    verificarReal("tolerancia predeterminada", 0.70,
                  configuracion.tolerancias[CLASE_ALTA][CLASE_ALTA]);

    if (!validarConfiguracion(&configuracion))
    {
        fprintf(stderr, "la configuracion predeterminada deberia ser valida\n");
        cantidadFallos++;
    }
}

static void archivoSobrescribeValores(void)
{
    Configuracion configuracion;

    iniciarConfiguracionPredeterminada(&configuracion);
    configuracion.tolerancias[CLASE_ALTA][CLASE_ALTA] = 0.0;

    if (!cargarConfiguracion(RUTA_CONFIGURACION_PRUEBA, &configuracion))
    {
        fprintf(stderr, "no se pudo cargar la configuracion de prueba\n");
        cantidadFallos++;
        return;
    }

    verificarEntero("ancho cargado", 8, configuracion.ancho);
    verificarEntero("alto cargado", 6, configuracion.alto);
    verificarEntero("iteraciones cargadas", 2, configuracion.iteraciones);
    verificarEntero("checkpoint cargado", 0, configuracion.frecuenciaCheckpoint);
    verificarReal("rho cargado", 0.30, configuracion.rho);
    verificarReal("proporcion residencial cargada", 0.90, configuracion.proporcionResidencial);
    verificarReal("proporcion de ocupacion cargada", 0.75, configuracion.proporcionOcupacion);
    verificarEntero("cantidad de zonas cargada", 2, configuracion.cantidadZonas);
    verificarReal("tolerancia cargada", 0.70, configuracion.tolerancias[CLASE_ALTA][CLASE_ALTA]);

    if (configuracion.ruidoHabilitado)
    {
        fprintf(stderr, "el ruido deberia estar deshabilitado\n");
        cantidadFallos++;
    }
}

int main(void)
{
    valoresPredeterminadosSonValidos();
    archivoSobrescribeValores();

    if (cantidadFallos != 0)
    {
        fprintf(stderr, "fallaron %d verificaciones\n", cantidadFallos);
        return 1;
    }

    printf("pruebas de configuracion correctas\n");
    return 0;
}
