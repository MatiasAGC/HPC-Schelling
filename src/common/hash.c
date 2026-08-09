#include "schelling/hash.h"

#include <stddef.h>
#include <string.h>

static uint64_t agregarBytes(uint64_t hash, const void *datos, size_t cantidad)
{
    const unsigned char *bytes = datos;

    for (size_t indice = 0; indice < cantidad; indice++)
    {
        hash ^= bytes[indice];
        hash *= UINT64_C(1099511628211);
    }

    return hash;
}

static uint64_t agregarEntero(uint64_t hash, uint64_t valor)
{
    for (int desplazamiento = 0; desplazamiento < 64; desplazamiento += 8)
    {
        unsigned char byte = (unsigned char)(valor >> desplazamiento);
        hash = agregarBytes(hash, &byte, 1);
    }

    return hash;
}

static uint64_t agregarReal(uint64_t hash, double valor)
{
    uint64_t bits;

    memcpy(&bits, &valor, sizeof(bits));
    return agregarEntero(hash, bits);
}

uint64_t calcularHashModelo(const Modelo *modelo)
{
    uint64_t hash = UINT64_C(14695981039346656037);

    hash = agregarEntero(hash, (uint64_t)modelo->ancho);
    hash = agregarEntero(hash, (uint64_t)modelo->alto);
    hash = agregarEntero(hash, (uint64_t)modelo->cantidadHogares);

    for (int idCelda = 0; idCelda < modelo->cantidadCeldas; idCelda++)
    {
        const Celda *celda = &modelo->celdas[idCelda];
        hash = agregarEntero(hash, (uint64_t)celda->tipo);
        hash = agregarEntero(hash, (uint64_t)(celda->idHogar + 1));
        hash = agregarEntero(hash, (uint64_t)celda->zona);
        hash = agregarReal(hash, celda->precio);
    }

    for (int idHogar = 0; idHogar < modelo->cantidadHogares; idHogar++)
    {
        const Hogar *hogar = &modelo->hogares[idHogar];
        hash = agregarEntero(hash, (uint64_t)hogar->id);
        hash = agregarEntero(hash, (uint64_t)hogar->idCelda);
        hash = agregarEntero(hash, (uint64_t)hogar->subestrato);
        hash = agregarEntero(hash, (uint64_t)hogar->clase);
        hash = agregarReal(hash, hogar->ingresoMensual);
        hash = agregarEntero(hash, (uint64_t)hogar->mesesBloqueado);
        hash = agregarEntero(hash, hogar->satisfecho ? 1 : 0);
    }

    return hash;
}

uint64_t calcularHashConfiguracionModelo(const Configuracion *configuracion)
{
    uint64_t hash = UINT64_C(14695981039346656037);

    hash = agregarEntero(hash, (uint64_t)configuracion->ancho);
    hash = agregarEntero(hash, (uint64_t)configuracion->alto);
    hash = agregarEntero(hash, (uint64_t)configuracion->radioVecindario);
    hash = agregarEntero(hash, configuracion->semilla);
    hash = agregarReal(hash, configuracion->sigma);
    hash = agregarEntero(hash, (uint64_t)configuracion->permanenciaMinima);
    hash = agregarReal(hash, configuracion->alpha0);
    hash = agregarReal(hash, configuracion->beta1);
    hash = agregarReal(hash, configuracion->beta2);
    hash = agregarReal(hash, configuracion->rho);
    hash = agregarReal(hash, configuracion->desviacionRuido);
    hash = agregarReal(hash, configuracion->limiteRuido);
    hash = agregarEntero(hash, configuracion->ruidoHabilitado ? 1 : 0);
    hash = agregarReal(hash, configuracion->fraccionFinanciada);
    hash = agregarReal(hash, configuracion->tasaAnual);
    hash = agregarEntero(hash, (uint64_t)configuracion->plazoMeses);
    hash = agregarReal(hash, configuracion->proporcionResidencial);
    hash = agregarReal(hash, configuracion->proporcionOcupacion);
    hash = agregarEntero(hash, (uint64_t)configuracion->cantidadZonas);
    hash = agregarEntero(hash, (uint64_t)configuracion->tamanoBloqueVacantes);

    for (int subestrato = 0; subestrato < CANTIDAD_SUBESTRATOS; subestrato++)
    {
        hash = agregarReal(hash, configuracion->pesosSubestratos[subestrato]);
    }

    for (int clase = 0; clase < CANTIDAD_CLASES; clase++)
    {
        for (int claseVecina = 0; claseVecina < CANTIDAD_CLASES; claseVecina++)
        {
            hash = agregarReal(hash, configuracion->tolerancias[clase][claseVecina]);
        }
    }

    return hash;
}
