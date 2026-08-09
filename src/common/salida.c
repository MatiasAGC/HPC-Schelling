#include "schelling/salida.h"

#include "schelling/registro.h"

#include <errno.h>
#include <inttypes.h>
#include <string.h>
#include <sys/stat.h>

bool crearRutaSalida(const SalidaEjecucion *salida, const char *nombre, char *ruta,
                     size_t longitudRuta)
{
    int escritos = snprintf(ruta, longitudRuta, "%s/%s", salida->directorio, nombre);
    return escritos >= 0 && (size_t)escritos < longitudRuta;
}

bool iniciarSalida(SalidaEjecucion *salida, const char *directorio,
                   const Configuracion *configuracion, const char *version, int procesos,
                   int threads)
{
    char ruta[1024];
    FILE *resumen;

    if (salida == NULL || directorio == NULL || strlen(directorio) >= sizeof(salida->directorio))
    {
        return false;
    }

    memset(salida, 0, sizeof(*salida));
    memcpy(salida->directorio, directorio, strlen(directorio) + 1);

    if (mkdir(directorio, 0775) != 0 && errno != EEXIST)
    {
        registrarError("no se pudo crear el directorio de salida %s", directorio);
        return false;
    }

    if (!crearRutaSalida(salida, "run.json", ruta, sizeof(ruta)))
    {
        return false;
    }

    resumen = fopen(ruta, "w");

    if (resumen == NULL)
    {
        return false;
    }

    fprintf(resumen,
            "{\n  \"version\": \"%s\",\n  \"ancho\": %d,\n  \"alto\": %d,\n"
            "  \"iteraciones\": %d,\n  \"semilla\": %" PRIu64 ",\n"
            "  \"radioVecindario\": %d,\n  \"sigma\": %.17g,\n"
            "  \"permanenciaMinima\": %d,\n  \"frecuenciaCheckpoint\": %d,\n"
            "  \"cantidadCheckpoints\": %d,\n  \"alpha0\": %.17g,\n"
            "  \"beta1\": %.17g,\n  \"beta2\": %.17g,\n  \"rho\": %.17g,\n"
            "  \"desviacionRuido\": %.17g,\n  \"limiteRuido\": %.17g,\n"
            "  \"ruidoHabilitado\": %s,\n  \"fraccionFinanciada\": %.17g,\n"
            "  \"tasaAnual\": %.17g,\n  \"plazoMeses\": %d,\n"
            "  \"proporcionResidencial\": %.17g,\n  \"proporcionOcupacion\": %.17g,\n"
            "  \"cantidadZonas\": %d,\n"
            "  \"pesosSubestratos\": [%.17g, %.17g, %.17g, %.17g, %.17g, %.17g, %.17g],\n"
            "  \"tolerancias\": [[%.17g, %.17g, %.17g], [%.17g, %.17g, %.17g], [%.17g, %.17g, "
            "%.17g]],\n"
            "  \"procesos\": %d,\n  \"threads\": %d\n}\n",
            version, configuracion->ancho, configuracion->alto, configuracion->iteraciones,
            configuracion->semilla, configuracion->radioVecindario, configuracion->sigma,
            configuracion->permanenciaMinima, configuracion->frecuenciaCheckpoint,
            configuracion->cantidadCheckpoints, configuracion->alpha0, configuracion->beta1,
            configuracion->beta2, configuracion->rho, configuracion->desviacionRuido,
            configuracion->limiteRuido, configuracion->ruidoHabilitado ? "true" : "false",
            configuracion->fraccionFinanciada, configuracion->tasaAnual, configuracion->plazoMeses,
            configuracion->proporcionResidencial, configuracion->proporcionOcupacion,
            configuracion->cantidadZonas, configuracion->pesosSubestratos[0],
            configuracion->pesosSubestratos[1], configuracion->pesosSubestratos[2],
            configuracion->pesosSubestratos[3], configuracion->pesosSubestratos[4],
            configuracion->pesosSubestratos[5], configuracion->pesosSubestratos[6],
            configuracion->tolerancias[0][0], configuracion->tolerancias[0][1],
            configuracion->tolerancias[0][2], configuracion->tolerancias[1][0],
            configuracion->tolerancias[1][1], configuracion->tolerancias[1][2],
            configuracion->tolerancias[2][0], configuracion->tolerancias[2][1],
            configuracion->tolerancias[2][2], procesos, threads);

    if (fclose(resumen) != 0 || !crearRutaSalida(salida, "metrics.csv", ruta, sizeof(ruta)))
    {
        return false;
    }

    salida->metricas = fopen(ruta, "w");

    if (salida->metricas == NULL || !crearRutaSalida(salida, "timings.csv", ruta, sizeof(ruta)))
    {
        cerrarSalida(salida);
        return false;
    }

    salida->tiempos = fopen(ruta, "w");

    if (salida->tiempos == NULL)
    {
        cerrarSalida(salida);
        return false;
    }

    fprintf(salida->metricas, "iteracion,satisfechos,aislados,bloqueados,solicitudes,aceptadas,"
                              "rechazadas,sinDestino,hash\n");
    fprintf(salida->tiempos, "iteracion,segundos\n");
    return true;
}

bool escribirMetricas(SalidaEjecucion *salida, uint64_t iteracion,
                      const MetricasIteracion *metricas, uint64_t hash)
{
    return fprintf(salida->metricas, "%" PRIu64 ",%d,%d,%d,%d,%d,%d,%d,%016" PRIx64 "\n", iteracion,
                   metricas->satisfechos, metricas->aislados, metricas->insatisfechosBloqueados,
                   metricas->solicitudes, metricas->aceptadas, metricas->rechazadas,
                   metricas->sinDestino, hash) > 0;
}

bool escribirTiempo(SalidaEjecucion *salida, uint64_t iteracion, double segundos)
{
    return fprintf(salida->tiempos, "%" PRIu64 ",%.9f\n", iteracion, segundos) > 0;
}

void cerrarSalida(SalidaEjecucion *salida)
{
    if (salida == NULL)
    {
        return;
    }

    if (salida->metricas != NULL)
    {
        fclose(salida->metricas);
    }
    if (salida->tiempos != NULL)
    {
        fclose(salida->tiempos);
    }

    salida->metricas = NULL;
    salida->tiempos = NULL;
}
