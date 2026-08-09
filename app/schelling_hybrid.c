#include "schelling/argumentos.h"
#include "schelling/configuracion.h"
#include "schelling/registro.h"

#include <mpi.h>
#include <omp.h>
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
    int soporteSolicitado = MPI_THREAD_FUNNELED;
    int soporteProvisto = MPI_THREAD_SINGLE;
    int rank = 0;
    int size = 0;
    int configuracionValida;
    int configuracionesValidas;
    int cantidadThreads = 1;

    MPI_Init_thread(&argc, &argv, soporteSolicitado, &soporteProvisto);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    iniciarOpcionesPrograma(&opciones);
    configuracionValida = leerArgumentos(argc, argv, &opciones) ? 1 : 0;

    if (configuracionValida != 0 && !opciones.mostrarAyuda && !opciones.mostrarVersion)
    {
        iniciarConfiguracionPredeterminada(&configuracion);
        configuracionValida =
            cargarConfiguracion(opciones.rutaConfiguracion, &configuracion) ? 1 : 0;

        if (configuracionValida != 0)
        {
            aplicarOpciones(&opciones, &configuracion);
            configuracionValida = validarConfiguracion(&configuracion) ? 1 : 0;
        }
    }

    MPI_Allreduce(&configuracionValida, &configuracionesValidas, 1, MPI_INT, MPI_SUM,
                  MPI_COMM_WORLD);

    if (configuracionesValidas != size)
    {
        if (rank == 0)
        {
            registrarError("al menos un proceso no pudo iniciar");
        }

        MPI_Finalize();
        return 1;
    }

    if (opciones.mostrarAyuda)
    {
        if (rank == 0)
        {
            mostrarUso(argv[0]);
        }

        MPI_Finalize();
        return 0;
    }

    if (opciones.mostrarVersion)
    {
        if (rank == 0)
        {
            printf("schelling_hybrid %s\n", SCHELLING_VERSION);
        }

        MPI_Finalize();
        return 0;
    }

    if (soporteProvisto < soporteSolicitado)
    {
        if (rank == 0)
        {
            registrarError("mpi no provee soporte MPI_THREAD_FUNNELED");
        }

        MPI_Finalize();
        return 1;
    }

#pragma omp parallel
    {
#pragma omp single
        {
            cantidadThreads = omp_get_num_threads();
        }
    }

    if (rank == 0)
    {
        registrarInformacion("inicio de la version hibrida");
        registrarInformacion("procesos mpi %d threads por proceso %d", size, cantidadThreads);
        mostrarConfiguracion(&configuracion);
        registrarInformacion("hito 0 completado sin ejecutar la simulacion");
    }

    MPI_Finalize();
    return 0;
}
