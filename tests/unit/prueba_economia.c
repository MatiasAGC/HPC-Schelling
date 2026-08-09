#include "schelling/configuracion.h"
#include "schelling/economia.h"
#include "schelling/modelo.h"
#include "schelling/vecindario.h"

#include <math.h>
#include <stdbool.h>
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

static void verificarReal(double esperado, double obtenido, double tolerancia, const char *mensaje)
{
    if (fabs(esperado - obtenido) > tolerancia)
    {
        fprintf(stderr, "%s esperado %.12f obtenido %.12f\n", mensaje, esperado, obtenido);
        cantidadFallos++;
    }
}

static void cuotaYAccesibilidadSonCorrectas(void)
{
    Configuracion configuracion;
    Hogar hogar = {0};
    double precio = 1448.150506;

    iniciarConfiguracionPredeterminada(&configuracion);
    hogar.ingresoMensual = obtenerIngresoBase(SUBESTRATO_M);
    verificarReal(8.3, calcularCuotaMensual(precio, &configuracion), 0.01,
                  "la cuota de referencia es incorrecta");
    verificar(esViviendaAccesible(precio, &hogar, &configuracion),
              "la vivienda de referencia deberia ser accesible");
    verificar(!esViviendaAccesible(precio * 2.0, &hogar, &configuracion),
              "la vivienda duplicada no deberia ser accesible");

    configuracion.tasaAnual = 0.0;
    verificarReal(configuracion.fraccionFinanciada * precio / configuracion.plazoMeses,
                  calcularCuotaMensual(precio, &configuracion), 1e-12,
                  "la cuota sin interes es incorrecta");
}

static void entornoYPrecioSonCorrectos(void)
{
    Configuracion configuracion;
    Modelo modelo = {0};
    Vecindario vecindario = {0};
    double demanda;
    double poder;
    double precioEsperado;

    iniciarConfiguracionPredeterminada(&configuracion);
    configuracion.ruidoHabilitado = false;
    verificar(crearModelo(&modelo, 3, 3, 2), "no se pudo crear el modelo economico");
    verificar(crearVecindario(&vecindario, 1, 1.0), "no se pudo crear el vecindario economico");
    verificar(ubicarHogar(&modelo, 0, 1, SUBESTRATO_A_MAS, 122.5),
              "no se pudo ubicar el hogar alto");
    verificar(ubicarHogar(&modelo, 1, 7, SUBESTRATO_B_MENOS, 9.9),
              "no se pudo ubicar el hogar bajo");
    verificar(definirCelda(&modelo, 0, CELDA_NO_RESIDENCIAL, 0, 0.0),
              "no se pudo excluir la esquina");
    verificar(definirCelda(&modelo, 2, CELDA_NO_RESIDENCIAL, 0, 0.0),
              "no se pudo excluir la segunda esquina");
    verificar(definirCelda(&modelo, 6, CELDA_NO_RESIDENCIAL, 0, 0.0),
              "no se pudo excluir la tercera esquina");
    verificar(definirCelda(&modelo, 8, CELDA_NO_RESIDENCIAL, 0, 0.0),
              "no se pudo excluir la cuarta esquina");
    verificar(calcularEntornoEconomico(&modelo, &vecindario, 4, &demanda, &poder),
              "no se pudo calcular el entorno economico");
    verificarReal(0.5, demanda, 1e-12, "la demanda es incorrecta");
    verificarReal(0.5, poder, 1e-12, "el poder adquisitivo es incorrecto");

    precioEsperado =
        exp(configuracion.alpha0 + configuracion.beta1 * 0.5 + configuracion.beta2 * 0.5);
    verificar(actualizarPreciosVacios(&modelo, &vecindario, &configuracion, 0),
              "no se pudieron actualizar los precios");
    verificarReal(precioEsperado, modelo.celdas[4].precio, 1e-9,
                  "el precio del centro es incorrecto");
    liberarVecindario(&vecindario);
    liberarModelo(&modelo);
}

int main(void)
{
    cuotaYAccesibilidadSonCorrectas();
    entornoYPrecioSonCorrectos();

    if (cantidadFallos != 0)
    {
        fprintf(stderr, "fallaron %d verificaciones\n", cantidadFallos);
        return 1;
    }

    printf("pruebas economicas correctas\n");
    return 0;
}
