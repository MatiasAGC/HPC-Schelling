#include "schelling/argumentos.h"
#include "schelling/configuracion.h"
#include "schelling/generador.h"
#include "schelling/modelo.h"
#include "schelling/registro.h"
#include "schelling/simulacion.h"
#include "schelling/vecindario.h"

#include <stdio.h>

static void aplicarOpciones(const OpcionesPrograma *opciones, Configuracion *configuracion)
{
    if (opciones->sobrescribirSemilla)
    {
        configuracion->semilla = opciones->semilla;
    }

    if (opciones->sobrescribirIteraciones)
    {
        configuracion->iteraciones = opciones->iteraciones;
    }

    if (opciones->sobrescribirCheckpoint)
    {
        configuracion->frecuenciaCheckpoint = opciones->frecuenciaCheckpoint;
    }
}

int main(int argc, char **argv)
{
    OpcionesPrograma opciones;
    Configuracion configuracion;
    Modelo modelo = {0};
    Vecindario vecindario = {0};

    iniciarOpcionesPrograma(&opciones);

    if (!leerArgumentos(argc, argv, &opciones))
    {
        mostrarUso(argv[0]);
        return 1;
    }

    if (opciones.mostrarAyuda)
    {
        mostrarUso(argv[0]);
        return 0;
    }

    if (opciones.mostrarVersion)
    {
        printf("schelling_seq %s\n", SCHELLING_VERSION);
        return 0;
    }

    iniciarConfiguracionPredeterminada(&configuracion);

    if (!cargarConfiguracion(opciones.rutaConfiguracion, &configuracion))
    {
        return 1;
    }

    aplicarOpciones(&opciones, &configuracion);

    if (!validarConfiguracion(&configuracion))
    {
        return 1;
    }

    registrarInformacion("inicio de la version secuencial");
    mostrarConfiguracion(&configuracion);

    if (!generarModeloSintetico(&configuracion, &modelo))
    {
        return 1;
    }

    registrarInformacion("estado sintetico con %d celdas %d viviendas %d hogares y %d vacias",
                         modelo.cantidadCeldas, contarCeldasResidenciales(&modelo),
                         modelo.cantidadHogares, contarViviendasVacias(&modelo));
    if (!crearVecindario(&vecindario, configuracion.radioVecindario, configuracion.sigma))
    {
        liberarModelo(&modelo);
        return 1;
    }

    for (int iteracion = 0; iteracion < configuracion.iteraciones; iteracion++)
    {
        MetricasIteracion metricas;

        if (!ejecutarIteracion(&modelo, &vecindario, &configuracion, (uint64_t)iteracion,
                               &metricas))
        {
            liberarVecindario(&vecindario);
            liberarModelo(&modelo);
            return 1;
        }

        registrarInformacion(
            "iteracion %d satisfechos %d solicitudes %d aceptadas %d rechazadas %d sin destino %d",
            iteracion + 1, metricas.satisfechos, metricas.solicitudes, metricas.aceptadas,
            metricas.rechazadas, metricas.sinDestino);
    }

    registrarInformacion("simulacion secuencial completada");
    liberarVecindario(&vecindario);
    liberarModelo(&modelo);
    return 0;
}
