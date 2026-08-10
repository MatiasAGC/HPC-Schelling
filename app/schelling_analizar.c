#include "schelling/analisis.h"
#include "schelling/checkpoint.h"
#include "schelling/configuracion.h"
#include "schelling/generador.h"
#include "schelling/modelo.h"
#include "schelling/vecindario.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

static void mostrarUso(const char *programa)
{
    printf("uso: %s --config archivo --state estado_final.bin --output prefijo\n", programa);
}

static bool leerOpciones(int argc, char **argv, const char **rutaConfiguracion,
                         const char **rutaEstado, const char **prefijoSalida)
{
    for (int indice = 1; indice < argc; indice++)
    {
        if (indice + 1 >= argc)
        {
            return false;
        }

        if (strcmp(argv[indice], "--config") == 0)
        {
            *rutaConfiguracion = argv[++indice];
        }
        else if (strcmp(argv[indice], "--state") == 0)
        {
            *rutaEstado = argv[++indice];
        }
        else if (strcmp(argv[indice], "--output") == 0)
        {
            *prefijoSalida = argv[++indice];
        }
        else
        {
            return false;
        }
    }

    return *rutaConfiguracion != NULL && *rutaEstado != NULL && *prefijoSalida != NULL;
}

static bool escribirResultados(const char *ruta, const MetricasSegregacion *inicial,
                               const MetricasSegregacion *final)
{
    FILE *archivo = fopen(ruta, "w");

    if (archivo == NULL)
    {
        return false;
    }

    bool correcto =
        fprintf(archivo,
                "estado,proporcionMismaClase,hogaresConVecinos,aislados\n"
                "inicial,%.9f,%d,%d\n"
                "final,%.9f,%d,%d\n",
                inicial->proporcionMismaClase, inicial->hogaresConVecinos, inicial->hogaresAislados,
                final->proporcionMismaClase, final->hogaresConVecinos, final->hogaresAislados) > 0;
    correcto = fclose(archivo) == 0 && correcto;
    return correcto;
}

int main(int argc, char **argv)
{
    const char *rutaConfiguracion = NULL;
    const char *rutaEstado = NULL;
    const char *prefijoSalida = NULL;
    Configuracion configuracion;
    Modelo inicial = {0};
    Modelo final = {0};
    Vecindario vecindario = {0};
    MetricasSegregacion metricasInicial;
    MetricasSegregacion metricasFinal;
    uint64_t iteracion;
    char rutaInicial[1024];
    char rutaFinal[1024];
    char rutaMetricas[1024];

    if (!leerOpciones(argc, argv, &rutaConfiguracion, &rutaEstado, &prefijoSalida))
    {
        mostrarUso(argv[0]);
        return 1;
    }

    iniciarConfiguracionPredeterminada(&configuracion);

    if (!cargarConfiguracion(rutaConfiguracion, &configuracion) ||
        !validarConfiguracion(&configuracion) ||
        !generarModeloSintetico(&configuracion, &inicial) ||
        !cargarCheckpoint(rutaEstado, &final, &configuracion, &iteracion) ||
        !crearVecindario(&vecindario, configuracion.radioVecindario, configuracion.sigma) ||
        !calcularMetricasSegregacion(&inicial, &vecindario, &metricasInicial) ||
        !calcularMetricasSegregacion(&final, &vecindario, &metricasFinal))
    {
        liberarVecindario(&vecindario);
        liberarModelo(&final);
        liberarModelo(&inicial);
        return 1;
    }

    snprintf(rutaInicial, sizeof(rutaInicial), "%s_inicial.ppm", prefijoSalida);
    snprintf(rutaFinal, sizeof(rutaFinal), "%s_final.ppm", prefijoSalida);
    snprintf(rutaMetricas, sizeof(rutaMetricas), "%s_metricas.csv", prefijoSalida);

    if (!escribirGrillaPpm(rutaInicial, &inicial) || !escribirGrillaPpm(rutaFinal, &final) ||
        !escribirResultados(rutaMetricas, &metricasInicial, &metricasFinal))
    {
        liberarVecindario(&vecindario);
        liberarModelo(&final);
        liberarModelo(&inicial);
        return 1;
    }

    printf("estado final en iteracion %" PRIu64 "\n", iteracion);
    printf("misma clase inicial %.3f final %.3f\n", metricasInicial.proporcionMismaClase,
           metricasFinal.proporcionMismaClase);
    liberarVecindario(&vecindario);
    liberarModelo(&final);
    liberarModelo(&inicial);
    return 0;
}
