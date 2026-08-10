#include "schelling/analisis.h"

#include <stdio.h>

bool calcularMetricasSegregacion(const Modelo *modelo, const Vecindario *vecindario,
                                 MetricasSegregacion *metricas)
{
    double suma = 0.0;

    if (modelo == NULL || vecindario == NULL || metricas == NULL)
    {
        return false;
    }

    metricas->hogaresConVecinos = 0;
    metricas->hogaresAislados = 0;

    for (int idHogar = 0; idHogar < modelo->cantidadHogares; idHogar++)
    {
        const Hogar *hogar = &modelo->hogares[idHogar];
        double proporciones[CANTIDAD_CLASES];
        bool aislado;

        if (!calcularProporciones(modelo, vecindario, hogar->idCelda, proporciones, &aislado))
        {
            return false;
        }

        if (aislado)
        {
            metricas->hogaresAislados++;
        }
        else
        {
            suma += proporciones[hogar->clase];
            metricas->hogaresConVecinos++;
        }
    }

    metricas->proporcionMismaClase =
        metricas->hogaresConVecinos > 0 ? suma / (double)metricas->hogaresConVecinos : 0.0;
    return true;
}

static void obtenerColor(const Modelo *modelo, const Celda *celda, unsigned char color[3])
{
    static const unsigned char coloresClase[CANTIDAD_CLASES][3] = {
        {213, 94, 0}, {0, 114, 178}, {0, 158, 115}};

    if (celda->tipo == CELDA_NO_RESIDENCIAL)
    {
        color[0] = 45;
        color[1] = 45;
        color[2] = 45;
    }
    else if (celda->idHogar == ID_INVALIDO)
    {
        color[0] = 238;
        color[1] = 238;
        color[2] = 238;
    }
    else
    {
        ClaseSocioeconomica clase = modelo->hogares[celda->idHogar].clase;
        color[0] = coloresClase[clase][0];
        color[1] = coloresClase[clase][1];
        color[2] = coloresClase[clase][2];
    }
}

bool escribirGrillaPpm(const char *ruta, const Modelo *modelo)
{
    FILE *archivo;
    bool correcto;

    if (ruta == NULL || modelo == NULL)
    {
        return false;
    }

    archivo = fopen(ruta, "wb");

    if (archivo == NULL)
    {
        return false;
    }

    correcto = fprintf(archivo, "P6\n%d %d\n255\n", modelo->ancho, modelo->alto) > 0;

    for (int idCelda = 0; correcto && idCelda < modelo->cantidadCeldas; idCelda++)
    {
        unsigned char color[3];
        obtenerColor(modelo, &modelo->celdas[idCelda], color);
        correcto = fwrite(color, sizeof(color), 1, archivo) == 1;
    }

    correcto = fclose(archivo) == 0 && correcto;
    return correcto;
}
