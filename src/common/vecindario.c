#include "schelling/vecindario.h"

#include "schelling/registro.h"

#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

bool crearVecindario(Vecindario *vecindario, int radio, double sigma)
{
    int lado;
    int cantidadMaxima;
    int indice = 0;
    DesplazamientoVecino *desplazamientos;

    if (vecindario == NULL || radio <= 0 || !isfinite(sigma) || sigma <= 0.0 ||
        radio > (INT32_MAX - 1) / 2)
    {
        return false;
    }

    lado = radio * 2 + 1;

    if (lado > INT32_MAX / lado)
    {
        return false;
    }

    cantidadMaxima = lado * lado - 1;

    if ((size_t)cantidadMaxima > SIZE_MAX / sizeof(DesplazamientoVecino))
    {
        return false;
    }

    desplazamientos = malloc((size_t)cantidadMaxima * sizeof(DesplazamientoVecino));

    if (desplazamientos == NULL)
    {
        registrarError("no se pudo reservar memoria para el vecindario");
        return false;
    }

    for (int deltaFila = -radio; deltaFila <= radio; deltaFila++)
    {
        for (int deltaColumna = -radio; deltaColumna <= radio; deltaColumna++)
        {
            double distanciaCuadrada;

            if (deltaFila == 0 && deltaColumna == 0)
            {
                continue;
            }

            distanciaCuadrada = (double)deltaFila * deltaFila + (double)deltaColumna * deltaColumna;
            desplazamientos[indice].deltaFila = deltaFila;
            desplazamientos[indice].deltaColumna = deltaColumna;
            desplazamientos[indice].peso = exp(-distanciaCuadrada / (2.0 * sigma * sigma));
            indice++;
        }
    }

    vecindario->radio = radio;
    vecindario->cantidadDesplazamientos = cantidadMaxima;
    vecindario->desplazamientos = desplazamientos;
    return true;
}

void liberarVecindario(Vecindario *vecindario)
{
    if (vecindario == NULL)
    {
        return;
    }

    free(vecindario->desplazamientos);
    vecindario->radio = 0;
    vecindario->cantidadDesplazamientos = 0;
    vecindario->desplazamientos = NULL;
}

bool calcularProporcionesExcluyendo(const Modelo *modelo, const Vecindario *vecindario, int idCelda,
                                    int idHogarExcluido, double proporciones[CANTIDAD_CLASES],
                                    bool *aislado)
{
    int fila;
    int columna;
    double pesoTotal = 0.0;

    if (modelo == NULL || vecindario == NULL || proporciones == NULL || aislado == NULL ||
        !obtenerCoordenadas(modelo, idCelda, &fila, &columna))
    {
        return false;
    }

    for (int clase = 0; clase < CANTIDAD_CLASES; clase++)
    {
        proporciones[clase] = 0.0;
    }

    for (int indice = 0; indice < vecindario->cantidadDesplazamientos; indice++)
    {
        const DesplazamientoVecino *desplazamiento = &vecindario->desplazamientos[indice];
        int idVecina = obtenerIdCelda(modelo, fila + desplazamiento->deltaFila,
                                      columna + desplazamiento->deltaColumna);

        if (idVecina != ID_INVALIDO)
        {
            const Celda *celdaVecina = &modelo->celdas[idVecina];

            if (celdaVecina->tipo == CELDA_RESIDENCIAL && celdaVecina->idHogar != ID_INVALIDO &&
                celdaVecina->idHogar != idHogarExcluido)
            {
                ClaseSocioeconomica clase = modelo->hogares[celdaVecina->idHogar].clase;

                proporciones[clase] += desplazamiento->peso;
                pesoTotal += desplazamiento->peso;
            }
        }
    }

    *aislado = pesoTotal == 0.0;

    if (!*aislado)
    {
        for (int clase = 0; clase < CANTIDAD_CLASES; clase++)
        {
            proporciones[clase] /= pesoTotal;
        }
    }

    return true;
}

bool calcularProporciones(const Modelo *modelo, const Vecindario *vecindario, int idCelda,
                          double proporciones[CANTIDAD_CLASES], bool *aislado)
{
    return calcularProporcionesExcluyendo(modelo, vecindario, idCelda, ID_INVALIDO, proporciones,
                                          aislado);
}

bool cumpleTolerancias(ClaseSocioeconomica clase, const double proporciones[CANTIDAD_CLASES],
                       const Configuracion *configuracion)
{
    const double *limites;

    if (clase < 0 || clase >= CANTIDAD_CLASES || proporciones == NULL || configuracion == NULL)
    {
        return false;
    }

    limites = configuracion->tolerancias[clase];

    if (clase == CLASE_ALTA)
    {
        return proporciones[CLASE_ALTA] >= limites[CLASE_ALTA] &&
               proporciones[CLASE_MEDIA] <= limites[CLASE_MEDIA] &&
               proporciones[CLASE_BAJA] <= limites[CLASE_BAJA];
    }

    if (clase == CLASE_MEDIA)
    {
        return proporciones[CLASE_ALTA] <= limites[CLASE_ALTA] &&
               proporciones[CLASE_MEDIA] >= limites[CLASE_MEDIA] &&
               proporciones[CLASE_BAJA] <= limites[CLASE_BAJA];
    }

    return proporciones[CLASE_ALTA] <= limites[CLASE_ALTA] &&
           proporciones[CLASE_MEDIA] <= limites[CLASE_MEDIA] &&
           proporciones[CLASE_BAJA] >= limites[CLASE_BAJA];
}

bool evaluarSatisfaccion(const Modelo *modelo, const Vecindario *vecindario, int idCelda,
                         ClaseSocioeconomica clase, const Configuracion *configuracion,
                         bool *aislado)
{
    double proporciones[CANTIDAD_CLASES];

    if (!calcularProporciones(modelo, vecindario, idCelda, proporciones, aislado))
    {
        return false;
    }

    return *aislado || cumpleTolerancias(clase, proporciones, configuracion);
}

bool evaluarSatisfaccionExcluyendo(const Modelo *modelo, const Vecindario *vecindario, int idCelda,
                                   int idHogarExcluido, ClaseSocioeconomica clase,
                                   const Configuracion *configuracion, bool *aislado)
{
    double proporciones[CANTIDAD_CLASES];

    if (!calcularProporcionesExcluyendo(modelo, vecindario, idCelda, idHogarExcluido, proporciones,
                                        aislado))
    {
        return false;
    }

    return *aislado || cumpleTolerancias(clase, proporciones, configuracion);
}
