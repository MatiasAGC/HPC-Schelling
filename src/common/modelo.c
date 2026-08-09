#include "schelling/modelo.h"

#include "schelling/registro.h"

#include <limits.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

static bool idCeldaValido(const Modelo *modelo, int idCelda)
{
    return idCelda >= 0 && idCelda < modelo->cantidadCeldas;
}

static bool idHogarValido(const Modelo *modelo, int idHogar)
{
    return idHogar >= 0 && idHogar < modelo->cantidadHogares;
}

bool crearModelo(Modelo *modelo, int ancho, int alto, int cantidadHogares)
{
    Celda *celdas;
    Hogar *hogares;
    size_t cantidadCeldas;

    if (modelo == NULL || ancho <= 0 || alto <= 0 || cantidadHogares < 0 || ancho > INT_MAX / alto)
    {
        registrarError("dimensiones o cantidad de hogares invalidas");
        return false;
    }

    cantidadCeldas = (size_t)ancho * (size_t)alto;

    if (cantidadCeldas > SIZE_MAX / sizeof(Celda) ||
        (size_t)cantidadHogares > SIZE_MAX / sizeof(Hogar))
    {
        registrarError("el modelo supera la memoria direccionable");
        return false;
    }

    celdas = calloc(cantidadCeldas, sizeof(Celda));
    hogares = calloc((size_t)cantidadHogares, sizeof(Hogar));

    if (celdas == NULL || (cantidadHogares > 0 && hogares == NULL))
    {
        registrarError("no se pudo reservar memoria para el modelo");
        free(celdas);
        free(hogares);
        return false;
    }

    modelo->ancho = ancho;
    modelo->alto = alto;
    modelo->cantidadCeldas = (int)cantidadCeldas;
    modelo->cantidadHogares = cantidadHogares;
    modelo->celdas = celdas;
    modelo->hogares = hogares;

    for (int idCelda = 0; idCelda < modelo->cantidadCeldas; idCelda++)
    {
        modelo->celdas[idCelda].tipo = CELDA_RESIDENCIAL;
        modelo->celdas[idCelda].idHogar = ID_INVALIDO;
    }

    for (int idHogar = 0; idHogar < modelo->cantidadHogares; idHogar++)
    {
        modelo->hogares[idHogar].id = idHogar;
        modelo->hogares[idHogar].idCelda = ID_INVALIDO;
    }

    return true;
}

void liberarModelo(Modelo *modelo)
{
    if (modelo == NULL)
    {
        return;
    }

    free(modelo->celdas);
    free(modelo->hogares);
    modelo->ancho = 0;
    modelo->alto = 0;
    modelo->cantidadCeldas = 0;
    modelo->cantidadHogares = 0;
    modelo->celdas = NULL;
    modelo->hogares = NULL;
}

int obtenerIdCelda(const Modelo *modelo, int fila, int columna)
{
    if (modelo == NULL || fila < 0 || fila >= modelo->alto || columna < 0 ||
        columna >= modelo->ancho)
    {
        return ID_INVALIDO;
    }

    return fila * modelo->ancho + columna;
}

bool obtenerCoordenadas(const Modelo *modelo, int idCelda, int *fila, int *columna)
{
    if (modelo == NULL || fila == NULL || columna == NULL || !idCeldaValido(modelo, idCelda))
    {
        return false;
    }

    *fila = idCelda / modelo->ancho;
    *columna = idCelda % modelo->ancho;
    return true;
}

bool definirCelda(Modelo *modelo, int idCelda, TipoCelda tipo, int zona, double precio)
{
    Celda *celda;

    if (modelo == NULL || !idCeldaValido(modelo, idCelda) ||
        (tipo != CELDA_RESIDENCIAL && tipo != CELDA_NO_RESIDENCIAL) || zona < 0 ||
        !isfinite(precio) || precio < 0.0)
    {
        return false;
    }

    celda = &modelo->celdas[idCelda];

    if (tipo == CELDA_NO_RESIDENCIAL && celda->idHogar != ID_INVALIDO)
    {
        return false;
    }

    celda->tipo = tipo;
    celda->zona = zona;
    celda->precio = precio;
    return true;
}

ClaseSocioeconomica obtenerClaseSubestrato(Subestrato subestrato)
{
    if (subestrato == SUBESTRATO_A_MAS || subestrato == SUBESTRATO_A_MENOS)
    {
        return CLASE_ALTA;
    }

    if (subestrato >= SUBESTRATO_M_MAS && subestrato <= SUBESTRATO_M_MENOS)
    {
        return CLASE_MEDIA;
    }

    return CLASE_BAJA;
}

double obtenerIngresoBase(Subestrato subestrato)
{
    static const double ingresos[CANTIDAD_SUBESTRATOS] = {122.5, 71.0, 46.7, 33.2, 23.7, 16.1, 9.9};

    if (subestrato < 0 || subestrato >= CANTIDAD_SUBESTRATOS)
    {
        return 0.0;
    }

    return ingresos[subestrato];
}

bool ubicarHogar(Modelo *modelo, int idHogar, int idCelda, Subestrato subestrato,
                 double ingresoMensual)
{
    Hogar *hogar;
    Celda *celda;

    if (modelo == NULL || !idHogarValido(modelo, idHogar) || !idCeldaValido(modelo, idCelda) ||
        subestrato < 0 || subestrato >= CANTIDAD_SUBESTRATOS || !isfinite(ingresoMensual) ||
        ingresoMensual <= 0.0)
    {
        return false;
    }

    hogar = &modelo->hogares[idHogar];
    celda = &modelo->celdas[idCelda];

    if (hogar->idCelda != ID_INVALIDO || celda->tipo != CELDA_RESIDENCIAL ||
        celda->idHogar != ID_INVALIDO)
    {
        return false;
    }

    hogar->idCelda = idCelda;
    hogar->subestrato = subestrato;
    hogar->clase = obtenerClaseSubestrato(subestrato);
    hogar->ingresoMensual = ingresoMensual;
    hogar->mesesBloqueado = 0;
    hogar->satisfecho = false;
    celda->idHogar = idHogar;
    return true;
}

int contarCeldasResidenciales(const Modelo *modelo)
{
    int cantidad = 0;

    if (modelo == NULL)
    {
        return 0;
    }

    for (int idCelda = 0; idCelda < modelo->cantidadCeldas; idCelda++)
    {
        if (modelo->celdas[idCelda].tipo == CELDA_RESIDENCIAL)
        {
            cantidad++;
        }
    }

    return cantidad;
}

int contarViviendasVacias(const Modelo *modelo)
{
    int cantidad = 0;

    if (modelo == NULL)
    {
        return 0;
    }

    for (int idCelda = 0; idCelda < modelo->cantidadCeldas; idCelda++)
    {
        const Celda *celda = &modelo->celdas[idCelda];

        if (celda->tipo == CELDA_RESIDENCIAL && celda->idHogar == ID_INVALIDO)
        {
            cantidad++;
        }
    }

    return cantidad;
}

bool validarModelo(const Modelo *modelo)
{
    if (modelo == NULL || modelo->ancho <= 0 || modelo->alto <= 0 || modelo->celdas == NULL ||
        (long long)modelo->cantidadCeldas != (long long)modelo->ancho * modelo->alto ||
        modelo->cantidadHogares < 0 || (modelo->cantidadHogares > 0 && modelo->hogares == NULL))
    {
        return false;
    }

    for (int idCelda = 0; idCelda < modelo->cantidadCeldas; idCelda++)
    {
        const Celda *celda = &modelo->celdas[idCelda];

        if (celda->tipo == CELDA_NO_RESIDENCIAL && celda->idHogar != ID_INVALIDO)
        {
            return false;
        }

        if (celda->idHogar != ID_INVALIDO)
        {
            if (!idHogarValido(modelo, celda->idHogar) ||
                modelo->hogares[celda->idHogar].idCelda != idCelda)
            {
                return false;
            }
        }
    }

    for (int idHogar = 0; idHogar < modelo->cantidadHogares; idHogar++)
    {
        const Hogar *hogar = &modelo->hogares[idHogar];

        if (hogar->id != idHogar || !idCeldaValido(modelo, hogar->idCelda) ||
            modelo->celdas[hogar->idCelda].idHogar != idHogar)
        {
            return false;
        }
    }

    return true;
}
