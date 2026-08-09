#include "schelling/argumentos.h"
#include "schelling/checkpoint.h"
#include "schelling/configuracion.h"
#include "schelling/generador.h"
#include "schelling/hash.h"
#include "schelling/registro.h"
#include "schelling/salida.h"
#include "schelling/simulacion_mpi.h"
#include "schelling/vecindario.h"

#include <inttypes.h>
#include <mpi.h>
#include <omp.h>
#include <stdio.h>
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

static bool todosExitosos(bool exito, MPI_Comm comunicador)
{
    int valorLocal = exito ? 1 : 0;
    int valorGlobal;
    MPI_Allreduce(&valorLocal, &valorGlobal, 1, MPI_INT, MPI_MIN, comunicador);
    return valorGlobal != 0;
}

int main(int argc, char **argv)
{
    OpcionesPrograma opciones;
    Configuracion configuracion;
    Modelo modelo = {0};
    SalidaEjecucion salida = {0};
    Vecindario vecindario = {0};
    uint64_t primeraIteracion = 0;
    int soporteProvisto = MPI_THREAD_SINGLE;
    int rank;
    int procesos;
    int cantidadThreads = 1;
    int codigo = 0;

    MPI_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED, &soporteProvisto);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &procesos);

    iniciarOpcionesPrograma(&opciones);
    bool inicioValido = leerArgumentos(argc, argv, &opciones);

    if (!todosExitosos(inicioValido, MPI_COMM_WORLD))
    {
        if (rank == 0)
        {
            mostrarUso(argv[0]);
        }
        codigo = 1;
        goto finalizar;
    }

    if (opciones.mostrarAyuda || opciones.mostrarVersion)
    {
        if (rank == 0)
        {
            if (opciones.mostrarAyuda)
            {
                mostrarUso(argv[0]);
            }
            else
            {
                printf("schelling_hybrid %s\n", SCHELLING_VERSION);
            }
        }
        goto finalizar;
    }

    iniciarConfiguracionPredeterminada(&configuracion);
    inicioValido = cargarConfiguracion(opciones.rutaConfiguracion, &configuracion);
    aplicarOpciones(&opciones, &configuracion);
    inicioValido = inicioValido && validarConfiguracion(&configuracion) &&
                   soporteProvisto >= MPI_THREAD_FUNNELED &&
                   procesos <= configuracion.alto / configuracion.radioVecindario;

    if (!todosExitosos(inicioValido, MPI_COMM_WORLD))
    {
        if (rank == 0)
        {
            registrarError("la configuracion mpi no es valida en todos los procesos");
        }
        codigo = 1;
        goto finalizar;
    }

#pragma omp parallel
    {
#pragma omp single
        {
            cantidadThreads = omp_get_num_threads();
        }
    }

    if (opciones.rutaReinicio[0] != '\0')
    {
        inicioValido =
            cargarCheckpoint(opciones.rutaReinicio, &modelo, &configuracion, &primeraIteracion) &&
            modelo.ancho == configuracion.ancho && modelo.alto == configuracion.alto;
    }
    else
    {
        inicioValido = generarModeloSintetico(&configuracion, &modelo);
    }

    inicioValido = inicioValido &&
                   crearVecindario(&vecindario, configuracion.radioVecindario, configuracion.sigma);

    if (!todosExitosos(inicioValido, MPI_COMM_WORLD))
    {
        codigo = 1;
        goto liberar;
    }

    if (rank == 0)
    {
        registrarInformacion("inicio de la version hibrida");
        registrarInformacion("procesos mpi %d threads por proceso %d", procesos, cantidadThreads);
        mostrarConfiguracion(&configuracion);
        registrarInformacion("estado con %d celdas %d viviendas %d hogares y %d vacias",
                             modelo.cantidadCeldas, contarCeldasResidenciales(&modelo),
                             modelo.cantidadHogares, contarViviendasVacias(&modelo));
        inicioValido = iniciarSalida(&salida, opciones.rutaSalida, &configuracion,
                                     SCHELLING_VERSION, procesos, cantidadThreads);
    }

    if (!todosExitosos(rank != 0 || inicioValido, MPI_COMM_WORLD))
    {
        codigo = 1;
        goto liberar;
    }

    for (uint64_t iteracion = primeraIteracion; iteracion < (uint64_t)configuracion.iteraciones;
         iteracion++)
    {
        MetricasIteracion metricas;
        int solicitudesRemotas;
        double inicio = MPI_Wtime();
        bool exito = ejecutarIteracionMpi(&modelo, &vecindario, &configuracion, iteracion,
                                          MPI_COMM_WORLD, &metricas, &solicitudesRemotas);
        double tiempoLocal = MPI_Wtime() - inicio;
        double tiempoGlobal;
        MPI_Allreduce(&tiempoLocal, &tiempoGlobal, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);

        uint64_t hash = calcularHashModelo(&modelo);
        uint64_t hashMinimo;
        uint64_t hashMaximo;
        MPI_Allreduce(&hash, &hashMinimo, 1, MPI_UINT64_T, MPI_MIN, MPI_COMM_WORLD);
        MPI_Allreduce(&hash, &hashMaximo, 1, MPI_UINT64_T, MPI_MAX, MPI_COMM_WORLD);
        exito = exito && hashMinimo == hashMaximo;

        if (rank == 0 && exito)
        {
            exito = escribirMetricas(&salida, iteracion + 1, &metricas, hash) &&
                    escribirTiempo(&salida, iteracion + 1, tiempoGlobal);
            registrarInformacion("iteracion %" PRIu64 " satisfechos %d solicitudes %d aceptadas "
                                 "%d rechazadas %d remotas %d hash %016" PRIx64,
                                 iteracion + 1, metricas.satisfechos, metricas.solicitudes,
                                 metricas.aceptadas, metricas.rechazadas, solicitudesRemotas, hash);

            if (exito && configuracion.frecuenciaCheckpoint > 0 &&
                (iteracion + 1) % (uint64_t)configuracion.frecuenciaCheckpoint == 0)
            {
                char nombre[64];
                char ruta[1024];
                snprintf(nombre, sizeof(nombre), "checkpoint_%06" PRIu64 ".bin", iteracion + 1);
                exito = crearRutaSalida(&salida, nombre, ruta, sizeof(ruta)) &&
                        guardarCheckpoint(ruta, &modelo, &configuracion, iteracion + 1);

                uint64_t antiguedad = (uint64_t)configuracion.frecuenciaCheckpoint *
                                      (uint64_t)configuracion.cantidadCheckpoints;

                if (exito && iteracion + 1 > antiguedad)
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

        if (!todosExitosos(rank != 0 || exito, MPI_COMM_WORLD))
        {
            codigo = 1;
            break;
        }
    }

    if (rank == 0 && codigo == 0)
    {
        char rutaFinal[1024];
        codigo = !crearRutaSalida(&salida, "estado_final.bin", rutaFinal, sizeof(rutaFinal)) ||
                         !guardarCheckpoint(rutaFinal, &modelo, &configuracion,
                                            (uint64_t)configuracion.iteraciones)
                     ? 1
                     : 0;
        if (codigo == 0)
        {
            registrarInformacion("simulacion hibrida completada");
        }
    }

liberar:
    if (rank == 0)
    {
        cerrarSalida(&salida);
    }
    liberarVecindario(&vecindario);
    liberarModelo(&modelo);

finalizar:
    MPI_Finalize();
    return codigo;
}
