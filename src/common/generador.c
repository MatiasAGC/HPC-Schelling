#include "schelling/generador.h"

#include "schelling/aleatorio.h"
#include "schelling/registro.h"

#include <math.h>
#include <stddef.h>
#include <stdint.h>

static bool esCeldaResidencial(const Configuracion *configuracion, int idCelda)
{
    return generarUniforme(configuracion->semilla, 0, (uint64_t)idCelda, PROPOSITO_TIPO_CELDA, 0) <
           configuracion->proporcionResidencial;
}

static bool estaCeldaOcupada(const Configuracion *configuracion, int idCelda)
{
    return generarUniforme(configuracion->semilla, 0, (uint64_t)idCelda, PROPOSITO_OCUPACION, 0) <
           configuracion->proporcionOcupacion;
}

static Subestrato elegirSubestrato(const Configuracion *configuracion, int idHogar)
{
    double sumaPesos = 0.0;
    double acumulado = 0.0;
    double seleccion;

    for (int subestrato = 0; subestrato < CANTIDAD_SUBESTRATOS; subestrato++)
    {
        sumaPesos += configuracion->pesosSubestratos[subestrato];
    }

    seleccion =
        generarUniforme(configuracion->semilla, 0, (uint64_t)idHogar, PROPOSITO_SUBESTRATO, 0) *
        sumaPesos;

    for (int subestrato = 0; subestrato < CANTIDAD_SUBESTRATOS; subestrato++)
    {
        acumulado += configuracion->pesosSubestratos[subestrato];

        if (seleccion < acumulado)
        {
            return (Subestrato)subestrato;
        }
    }

    return SUBESTRATO_B_MENOS;
}

static int obtenerZona(const Configuracion *configuracion, int columna)
{
    int64_t posicion = (int64_t)columna * configuracion->cantidadZonas;
    int zona = (int)(posicion / configuracion->ancho);

    if (zona >= configuracion->cantidadZonas)
    {
        zona = configuracion->cantidadZonas - 1;
    }

    return zona;
}

bool generarModeloSintetico(const Configuracion *configuracion, Modelo *modelo)
{
    int cantidadHogares = 0;
    int cantidadCeldas;
    int idHogar = 0;
    double precioInicial;

    if (configuracion == NULL || modelo == NULL || !validarConfiguracion(configuracion))
    {
        return false;
    }

    precioInicial = exp(configuracion->alpha0);
    cantidadCeldas = configuracion->ancho * configuracion->alto;

    if (!isfinite(precioInicial))
    {
        registrarError("el precio inicial sintetico no es representable");
        return false;
    }

    for (int idCelda = 0; idCelda < cantidadCeldas; idCelda++)
    {
        if (esCeldaResidencial(configuracion, idCelda) && estaCeldaOcupada(configuracion, idCelda))
        {
            cantidadHogares++;
        }
    }

    if (!crearModelo(modelo, configuracion->ancho, configuracion->alto, cantidadHogares))
    {
        return false;
    }

    for (int idCelda = 0; idCelda < modelo->cantidadCeldas; idCelda++)
    {
        int columna = idCelda % modelo->ancho;
        int zona = obtenerZona(configuracion, columna);
        bool residencial = esCeldaResidencial(configuracion, idCelda);

        if (!definirCelda(modelo, idCelda, residencial ? CELDA_RESIDENCIAL : CELDA_NO_RESIDENCIAL,
                          zona, residencial ? precioInicial : 0.0))
        {
            liberarModelo(modelo);
            return false;
        }

        if (residencial && estaCeldaOcupada(configuracion, idCelda))
        {
            Subestrato subestrato = elegirSubestrato(configuracion, idHogar);

            if (!ubicarHogar(modelo, idHogar, idCelda, subestrato, obtenerIngresoBase(subestrato)))
            {
                liberarModelo(modelo);
                return false;
            }

            idHogar++;
        }
    }

    return idHogar == cantidadHogares && validarModelo(modelo);
}
