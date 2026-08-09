#include "schelling/configuracion.h"
#include "schelling/generador.h"
#include "schelling/modelo.h"

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

static bool modelosSonIguales(const Modelo *primero, const Modelo *segundo)
{
    if (primero->cantidadCeldas != segundo->cantidadCeldas ||
        primero->cantidadHogares != segundo->cantidadHogares)
    {
        return false;
    }

    for (int idCelda = 0; idCelda < primero->cantidadCeldas; idCelda++)
    {
        const Celda *celdaPrimera = &primero->celdas[idCelda];
        const Celda *celdaSegunda = &segundo->celdas[idCelda];

        if (celdaPrimera->tipo != celdaSegunda->tipo ||
            celdaPrimera->idHogar != celdaSegunda->idHogar ||
            celdaPrimera->zona != celdaSegunda->zona ||
            celdaPrimera->precio != celdaSegunda->precio)
        {
            return false;
        }
    }

    for (int idHogar = 0; idHogar < primero->cantidadHogares; idHogar++)
    {
        const Hogar *hogarPrimero = &primero->hogares[idHogar];
        const Hogar *hogarSegundo = &segundo->hogares[idHogar];

        if (hogarPrimero->idCelda != hogarSegundo->idCelda ||
            hogarPrimero->subestrato != hogarSegundo->subestrato ||
            hogarPrimero->ingresoMensual != hogarSegundo->ingresoMensual)
        {
            return false;
        }
    }

    return true;
}

static void generacionEsReproducible(void)
{
    Configuracion configuracion;
    Modelo primero = {0};
    Modelo segundo = {0};

    iniciarConfiguracionPredeterminada(&configuracion);
    configuracion.ancho = 20;
    configuracion.alto = 10;
    configuracion.proporcionResidencial = 0.80;
    configuracion.proporcionOcupacion = 0.70;
    configuracion.cantidadZonas = 4;

    verificar(generarModeloSintetico(&configuracion, &primero),
              "no se pudo generar el primer modelo");
    verificar(generarModeloSintetico(&configuracion, &segundo),
              "no se pudo generar el segundo modelo");
    verificar(validarModelo(&primero), "el modelo sintetico deberia ser valido");
    verificar(modelosSonIguales(&primero, &segundo),
              "la misma semilla deberia reproducir el modelo");
    verificar(primero.cantidadHogares > 0 && primero.cantidadHogares < primero.cantidadCeldas,
              "la prueba deberia contener hogares y vacantes");

    for (int idCelda = 0; idCelda < primero.cantidadCeldas; idCelda++)
    {
        verificar(primero.celdas[idCelda].zona >= 0 && primero.celdas[idCelda].zona < 4,
                  "una zona sintetica esta fuera de rango");
    }

    liberarModelo(&primero);
    liberarModelo(&segundo);
}

static void ocupacionCompletaEsExacta(void)
{
    Configuracion configuracion;
    Modelo modelo = {0};

    iniciarConfiguracionPredeterminada(&configuracion);
    configuracion.ancho = 5;
    configuracion.alto = 4;
    configuracion.proporcionResidencial = 1.0;
    configuracion.proporcionOcupacion = 1.0;
    configuracion.cantidadZonas = 2;

    verificar(generarModeloSintetico(&configuracion, &modelo),
              "no se pudo generar el modelo completamente ocupado");
    verificar(modelo.cantidadCeldas == 20, "la cantidad de celdas es incorrecta");
    verificar(modelo.cantidadHogares == 20, "la ocupacion completa deberia ser exacta");
    verificar(contarCeldasResidenciales(&modelo) == 20,
              "todas las celdas deberian ser residenciales");
    verificar(contarViviendasVacias(&modelo) == 0, "no deberia haber viviendas vacias");
    liberarModelo(&modelo);
}

int main(void)
{
    generacionEsReproducible();
    ocupacionCompletaEsExacta();

    if (cantidadFallos != 0)
    {
        fprintf(stderr, "fallaron %d verificaciones\n", cantidadFallos);
        return 1;
    }

    printf("pruebas del generador correctas\n");
    return 0;
}
