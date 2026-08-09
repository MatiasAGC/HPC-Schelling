#include "schelling/argumentos.h"
#include "schelling/configuracion.h"
#include "schelling/registro.h"

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
    registrarInformacion("hito 0 completado sin ejecutar la simulacion");
    return 0;
}
