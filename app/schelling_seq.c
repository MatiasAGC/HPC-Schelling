#include "schelling/argumentos.h"
#include "schelling/checkpoint.h"
#include "schelling/configuracion.h"
#include "schelling/generador.h"
#include "schelling/hash.h"
#include "schelling/modelo.h"
#include "schelling/registro.h"
#include "schelling/salida.h"
#include "schelling/simulacion.h"
#include "schelling/vecindario.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

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
    SalidaEjecucion salida = {0};
    Vecindario vecindario = {0};
    uint64_t primeraIteracion = 0;

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

    if (opciones.rutaReinicio[0] != '\0')
    {
        if (!cargarCheckpoint(opciones.rutaReinicio, &modelo, &configuracion, &primeraIteracion) ||
            modelo.ancho != configuracion.ancho || modelo.alto != configuracion.alto)
        {
            registrarError("el checkpoint no es compatible con la configuracion");
            liberarModelo(&modelo);
            return 1;
        }
    }
    else if (!generarModeloSintetico(&configuracion, &modelo))
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

    if (!iniciarSalida(&salida, opciones.rutaSalida, &configuracion, SCHELLING_VERSION, 1, 1))
    {
        liberarVecindario(&vecindario);
        liberarModelo(&modelo);
        return 1;
    }

    for (uint64_t iteracion = primeraIteracion; iteracion < (uint64_t)configuracion.iteraciones;
         iteracion++)
    {
        MetricasIteracion metricas;
        struct timespec inicio;
        struct timespec fin;

        timespec_get(&inicio, TIME_UTC);

        if (!ejecutarIteracion(&modelo, &vecindario, &configuracion, iteracion, &metricas))
        {
            cerrarSalida(&salida);
            liberarVecindario(&vecindario);
            liberarModelo(&modelo);
            return 1;
        }

        timespec_get(&fin, TIME_UTC);
        double segundos =
            (double)(fin.tv_sec - inicio.tv_sec) + (double)(fin.tv_nsec - inicio.tv_nsec) / 1e9;
        uint64_t hash = calcularHashModelo(&modelo);

        if (!escribirMetricas(&salida, iteracion + 1, &metricas, hash) ||
            !escribirTiempo(&salida, iteracion + 1, segundos))
        {
            cerrarSalida(&salida);
            liberarVecindario(&vecindario);
            liberarModelo(&modelo);
            return 1;
        }

        registrarInformacion("iteracion %" PRIu64 " satisfechos %d solicitudes %d aceptadas %d "
                             "rechazadas %d sin destino %d hash %016" PRIx64,
                             iteracion + 1, metricas.satisfechos, metricas.solicitudes,
                             metricas.aceptadas, metricas.rechazadas, metricas.sinDestino, hash);

        if (configuracion.frecuenciaCheckpoint > 0 &&
            (iteracion + 1) % (uint64_t)configuracion.frecuenciaCheckpoint == 0)
        {
            char nombre[64];
            char ruta[1024];
            snprintf(nombre, sizeof(nombre), "checkpoint_%06" PRIu64 ".bin", iteracion + 1);

            if (!crearRutaSalida(&salida, nombre, ruta, sizeof(ruta)) ||
                !guardarCheckpoint(ruta, &modelo, &configuracion, iteracion + 1))
            {
                cerrarSalida(&salida);
                liberarVecindario(&vecindario);
                liberarModelo(&modelo);
                return 1;
            }

            uint64_t antiguedad = (uint64_t)configuracion.frecuenciaCheckpoint *
                                  (uint64_t)configuracion.cantidadCheckpoints;

            if (iteracion + 1 > antiguedad)
            {
                snprintf(nombre, sizeof(nombre), "checkpoint_%06" PRIu64 ".bin",
                         iteracion + 1 - antiguedad);

                if (crearRutaSalida(&salida, nombre, ruta, sizeof(ruta)))
                {
                    remove(ruta);
                }
            }
        }
    }

    char rutaFinal[1024];

    if (!crearRutaSalida(&salida, "estado_final.bin", rutaFinal, sizeof(rutaFinal)) ||
        !guardarCheckpoint(rutaFinal, &modelo, &configuracion, (uint64_t)configuracion.iteraciones))
    {
        cerrarSalida(&salida);
        liberarVecindario(&vecindario);
        liberarModelo(&modelo);
        return 1;
    }

    registrarInformacion("simulacion secuencial completada");
    cerrarSalida(&salida);
    liberarVecindario(&vecindario);
    liberarModelo(&modelo);
    return 0;
}
