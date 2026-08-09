#include "schelling/indice_vacantes.h"

#include "schelling/registro.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

int obtenerIdBloque(const IndiceVacantes *indice, int filaBloque, int columnaBloque)
{
    if (indice == NULL || filaBloque < 0 || filaBloque >= indice->bloquesAlto ||
        columnaBloque < 0 || columnaBloque >= indice->bloquesAncho)
    {
        return ID_INVALIDO;
    }

    return filaBloque * indice->bloquesAncho + columnaBloque;
}

bool crearIndiceVacantesFiltrado(IndiceVacantes *indice, const Modelo *modelo, int tamanoBloque,
                                 const unsigned char *incluir)
{
    int *posiciones;

    if (indice == NULL || modelo == NULL || tamanoBloque <= 0)
    {
        return false;
    }

    memset(indice, 0, sizeof(*indice));
    indice->tamanoBloque = tamanoBloque;
    indice->bloquesAncho = (modelo->ancho + tamanoBloque - 1) / tamanoBloque;
    indice->bloquesAlto = (modelo->alto + tamanoBloque - 1) / tamanoBloque;
    indice->cantidadBloques = indice->bloquesAncho * indice->bloquesAlto;
    indice->cantidadVacantes = 0;

    for (int idCelda = 0; idCelda < modelo->cantidadCeldas; idCelda++)
    {
        const Celda *celda = &modelo->celdas[idCelda];

        if (celda->tipo == CELDA_RESIDENCIAL && celda->idHogar == ID_INVALIDO &&
            (incluir == NULL || incluir[idCelda] != 0))
        {
            indice->cantidadVacantes++;
        }
    }
    indice->inicios = calloc((size_t)indice->cantidadBloques + 1, sizeof(int));
    indice->idsVacantes = malloc((size_t)indice->cantidadVacantes * sizeof(int));
    posiciones = malloc((size_t)indice->cantidadBloques * sizeof(int));

    if (indice->inicios == NULL || (indice->cantidadVacantes > 0 && indice->idsVacantes == NULL) ||
        posiciones == NULL)
    {
        free(posiciones);
        liberarIndiceVacantes(indice);
        registrarError("no se pudo reservar memoria para el indice de vacantes");
        return false;
    }

    for (int idCelda = 0; idCelda < modelo->cantidadCeldas; idCelda++)
    {
        const Celda *celda = &modelo->celdas[idCelda];

        if (celda->tipo == CELDA_RESIDENCIAL && celda->idHogar == ID_INVALIDO &&
            (incluir == NULL || incluir[idCelda] != 0))
        {
            int fila = idCelda / modelo->ancho;
            int columna = idCelda % modelo->ancho;
            int idBloque = obtenerIdBloque(indice, fila / tamanoBloque, columna / tamanoBloque);
            indice->inicios[idBloque + 1]++;
        }
    }

    for (int idBloque = 0; idBloque < indice->cantidadBloques; idBloque++)
    {
        indice->inicios[idBloque + 1] += indice->inicios[idBloque];
        posiciones[idBloque] = indice->inicios[idBloque];
    }

    for (int idCelda = 0; idCelda < modelo->cantidadCeldas; idCelda++)
    {
        const Celda *celda = &modelo->celdas[idCelda];

        if (celda->tipo == CELDA_RESIDENCIAL && celda->idHogar == ID_INVALIDO &&
            (incluir == NULL || incluir[idCelda] != 0))
        {
            int fila = idCelda / modelo->ancho;
            int columna = idCelda % modelo->ancho;
            int idBloque = obtenerIdBloque(indice, fila / tamanoBloque, columna / tamanoBloque);
            indice->idsVacantes[posiciones[idBloque]++] = idCelda;
        }
    }

    free(posiciones);
    return true;
}

bool crearIndiceVacantes(IndiceVacantes *indice, const Modelo *modelo, int tamanoBloque)
{
    return crearIndiceVacantesFiltrado(indice, modelo, tamanoBloque, NULL);
}

void liberarIndiceVacantes(IndiceVacantes *indice)
{
    if (indice == NULL)
    {
        return;
    }

    free(indice->inicios);
    free(indice->idsVacantes);
    memset(indice, 0, sizeof(*indice));
}
