#include "schelling/checkpoint.h"

#include "schelling/hash.h"
#include "schelling/registro.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VERSION_CHECKPOINT UINT32_C(1)
#define LONGITUD_TEMPORAL 1024

static const unsigned char firma[8] = {'S', 'C', 'H', 'E', 'L', 'L', '0', '1'};

static bool escribir(const void *datos, size_t tamano, size_t cantidad, FILE *archivo)
{
    return fwrite(datos, tamano, cantidad, archivo) == cantidad;
}

static bool leer(void *datos, size_t tamano, size_t cantidad, FILE *archivo)
{
    return fread(datos, tamano, cantidad, archivo) == cantidad;
}

bool guardarCheckpoint(const char *ruta, const Modelo *modelo, const Configuracion *configuracion,
                       uint64_t proximaIteracion)
{
    char rutaTemporal[LONGITUD_TEMPORAL];
    uint32_t version = VERSION_CHECKPOINT;
    uint32_t ancho;
    uint32_t alto;
    uint32_t cantidadHogares;
    uint64_t hash;
    uint64_t hashConfiguracion;
    FILE *archivo;
    bool correcto;

    if (ruta == NULL || modelo == NULL || configuracion == NULL ||
        strlen(ruta) + 5 >= sizeof(rutaTemporal))
    {
        return false;
    }

    ancho = (uint32_t)modelo->ancho;
    alto = (uint32_t)modelo->alto;
    cantidadHogares = (uint32_t)modelo->cantidadHogares;
    hash = calcularHashModelo(modelo);
    hashConfiguracion = calcularHashConfiguracionModelo(configuracion);

    snprintf(rutaTemporal, sizeof(rutaTemporal), "%s.tmp", ruta);
    archivo = fopen(rutaTemporal, "wb");

    if (archivo == NULL)
    {
        registrarError("no se pudo crear el checkpoint %s", rutaTemporal);
        return false;
    }

    correcto = escribir(firma, sizeof(firma), 1, archivo) &&
               escribir(&version, sizeof(version), 1, archivo) &&
               escribir(&proximaIteracion, sizeof(proximaIteracion), 1, archivo) &&
               escribir(&ancho, sizeof(ancho), 1, archivo) &&
               escribir(&alto, sizeof(alto), 1, archivo) &&
               escribir(&cantidadHogares, sizeof(cantidadHogares), 1, archivo) &&
               escribir(&hashConfiguracion, sizeof(hashConfiguracion), 1, archivo) &&
               escribir(&hash, sizeof(hash), 1, archivo);

    for (int idCelda = 0; correcto && idCelda < modelo->cantidadCeldas; idCelda++)
    {
        const Celda *celda = &modelo->celdas[idCelda];
        int32_t tipo = (int32_t)celda->tipo;
        int32_t idHogar = (int32_t)celda->idHogar;
        int32_t zona = (int32_t)celda->zona;
        correcto = escribir(&tipo, sizeof(tipo), 1, archivo) &&
                   escribir(&idHogar, sizeof(idHogar), 1, archivo) &&
                   escribir(&zona, sizeof(zona), 1, archivo) &&
                   escribir(&celda->precio, sizeof(celda->precio), 1, archivo);
    }

    for (int idHogar = 0; correcto && idHogar < modelo->cantidadHogares; idHogar++)
    {
        const Hogar *hogar = &modelo->hogares[idHogar];
        int32_t valores[5] = {(int32_t)hogar->id, (int32_t)hogar->idCelda,
                              (int32_t)hogar->subestrato, (int32_t)hogar->clase,
                              (int32_t)hogar->mesesBloqueado};
        uint8_t satisfecho = hogar->satisfecho ? UINT8_C(1) : UINT8_C(0);
        correcto = escribir(valores, sizeof(valores[0]), 5, archivo) &&
                   escribir(&hogar->ingresoMensual, sizeof(hogar->ingresoMensual), 1, archivo) &&
                   escribir(&satisfecho, sizeof(satisfecho), 1, archivo);
    }

    if (correcto)
    {
        correcto = fflush(archivo) == 0;
    }

    correcto = fclose(archivo) == 0 && correcto;

    if (!correcto || rename(rutaTemporal, ruta) != 0)
    {
        remove(rutaTemporal);
        registrarError("no se pudo publicar el checkpoint %s", ruta);
        return false;
    }

    return true;
}

bool cargarCheckpoint(const char *ruta, Modelo *modelo, const Configuracion *configuracion,
                      uint64_t *proximaIteracion)
{
    unsigned char firmaLeida[8];
    uint32_t version;
    uint32_t ancho;
    uint32_t alto;
    uint32_t cantidadHogares;
    uint64_t hashEsperado;
    uint64_t hashConfiguracion;
    FILE *archivo;
    bool correcto;

    if (ruta == NULL || modelo == NULL || configuracion == NULL || proximaIteracion == NULL)
    {
        return false;
    }

    archivo = fopen(ruta, "rb");

    if (archivo == NULL)
    {
        registrarError("no se pudo abrir el checkpoint %s", ruta);
        return false;
    }

    correcto = leer(firmaLeida, sizeof(firmaLeida), 1, archivo) &&
               leer(&version, sizeof(version), 1, archivo) &&
               leer(proximaIteracion, sizeof(*proximaIteracion), 1, archivo) &&
               leer(&ancho, sizeof(ancho), 1, archivo) && leer(&alto, sizeof(alto), 1, archivo) &&
               leer(&cantidadHogares, sizeof(cantidadHogares), 1, archivo) &&
               leer(&hashConfiguracion, sizeof(hashConfiguracion), 1, archivo) &&
               leer(&hashEsperado, sizeof(hashEsperado), 1, archivo);

    if (!correcto || memcmp(firmaLeida, firma, sizeof(firma)) != 0 ||
        version != VERSION_CHECKPOINT || ancho > INT_MAX || alto > INT_MAX ||
        cantidadHogares > INT_MAX ||
        hashConfiguracion != calcularHashConfiguracionModelo(configuracion) ||
        !crearModelo(modelo, (int)ancho, (int)alto, (int)cantidadHogares))
    {
        fclose(archivo);
        registrarError("cabecera de checkpoint invalida en %s", ruta);
        return false;
    }

    for (int idCelda = 0; correcto && idCelda < modelo->cantidadCeldas; idCelda++)
    {
        Celda *celda = &modelo->celdas[idCelda];
        int32_t tipo;
        int32_t idHogar;
        int32_t zona;
        correcto = leer(&tipo, sizeof(tipo), 1, archivo) &&
                   leer(&idHogar, sizeof(idHogar), 1, archivo) &&
                   leer(&zona, sizeof(zona), 1, archivo) &&
                   leer(&celda->precio, sizeof(celda->precio), 1, archivo);
        celda->tipo = (TipoCelda)tipo;
        celda->idHogar = (int)idHogar;
        celda->zona = (int)zona;
    }

    for (int idHogar = 0; correcto && idHogar < modelo->cantidadHogares; idHogar++)
    {
        Hogar *hogar = &modelo->hogares[idHogar];
        int32_t valores[5];
        uint8_t satisfecho;
        correcto = leer(valores, sizeof(valores[0]), 5, archivo) &&
                   leer(&hogar->ingresoMensual, sizeof(hogar->ingresoMensual), 1, archivo) &&
                   leer(&satisfecho, sizeof(satisfecho), 1, archivo);
        hogar->id = (int)valores[0];
        hogar->idCelda = (int)valores[1];
        hogar->subestrato = (Subestrato)valores[2];
        hogar->clase = (ClaseSocioeconomica)valores[3];
        hogar->mesesBloqueado = (int)valores[4];
        hogar->satisfecho = satisfecho != 0;
    }

    int byteAdicional = fgetc(archivo);
    bool finValido = byteAdicional == EOF && !ferror(archivo);
    bool cierreValido = fclose(archivo) == 0;
    correcto = correcto && finValido && cierreValido && validarModelo(modelo) &&
               calcularHashModelo(modelo) == hashEsperado;

    if (!correcto)
    {
        liberarModelo(modelo);
        registrarError("contenido de checkpoint invalido en %s", ruta);
        return false;
    }

    return true;
}
