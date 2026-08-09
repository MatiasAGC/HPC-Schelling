#include "schelling/configuracion.h"
#include "schelling/modelo.h"
#include "schelling/simulacion.h"
#include "schelling/vecindario.h"

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

static void conflictoTieneUnSoloGanador(void)
{
    Configuracion configuracion;
    Modelo modelo = {0};
    Vecindario vecindario = {0};
    MetricasIteracion metricas;

    iniciarConfiguracionPredeterminada(&configuracion);
    configuracion.alpha0 = 0.0;
    configuracion.beta1 = 0.0;
    configuracion.beta2 = 0.0;
    configuracion.ruidoHabilitado = false;
    configuracion.rho = 1.0;
    configuracion.permanenciaMinima = 2;

    verificar(crearModelo(&modelo, 5, 1, 4), "no se pudo crear el escenario de conflicto");
    verificar(crearVecindario(&vecindario, 1, 1.0), "no se pudo crear el vecindario de conflicto");
    verificar(ubicarHogar(&modelo, 0, 0, SUBESTRATO_B_MENOS, 9.9),
              "no se pudo ubicar el primer hogar bajo");
    verificar(ubicarHogar(&modelo, 1, 1, SUBESTRATO_A_MAS, 122.5),
              "no se pudo ubicar el primer hogar alto");
    verificar(ubicarHogar(&modelo, 2, 3, SUBESTRATO_A_MAS, 122.5),
              "no se pudo ubicar el segundo hogar alto");
    verificar(ubicarHogar(&modelo, 3, 4, SUBESTRATO_B_MENOS, 9.9),
              "no se pudo ubicar el segundo hogar bajo");
    modelo.hogares[0].mesesBloqueado = 1;
    modelo.hogares[3].mesesBloqueado = 1;

    verificar(ejecutarIteracion(&modelo, &vecindario, &configuracion, 0, &metricas),
              "no se pudo ejecutar la iteracion de conflicto");
    verificar(metricas.solicitudes == 2, "deberia haber dos solicitudes");
    verificar(metricas.aceptadas == 1, "deberia aceptarse una solicitud");
    verificar(metricas.rechazadas == 1, "deberia rechazarse una solicitud");
    verificar(modelo.celdas[2].idHogar == 1 || modelo.celdas[2].idHogar == 2,
              "la vacante deberia tener un ganador alto");
    verificar(contarViviendasVacias(&modelo) == 1,
              "una mudanza debe conservar la cantidad de vacantes");
    verificar(validarModelo(&modelo), "el modelo debe conservar sus invariantes");

    liberarVecindario(&vecindario);
    liberarModelo(&modelo);
}

int main(void)
{
    conflictoTieneUnSoloGanador();

    if (cantidadFallos != 0)
    {
        fprintf(stderr, "fallaron %d verificaciones\n", cantidadFallos);
        return 1;
    }

    printf("pruebas de simulacion correctas\n");
    return 0;
}
