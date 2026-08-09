#ifndef SCHELLING_MODELO_H
#define SCHELLING_MODELO_H

#include <stdbool.h>

#define ID_INVALIDO (-1)

typedef enum
{
    CELDA_NO_RESIDENCIAL,
    CELDA_RESIDENCIAL
} TipoCelda;

typedef enum
{
    CLASE_ALTA,
    CLASE_MEDIA,
    CLASE_BAJA,
    CANTIDAD_CLASES
} ClaseSocioeconomica;

typedef enum
{
    SUBESTRATO_A_MAS,
    SUBESTRATO_A_MENOS,
    SUBESTRATO_M_MAS,
    SUBESTRATO_M,
    SUBESTRATO_M_MENOS,
    SUBESTRATO_B_MAS,
    SUBESTRATO_B_MENOS,
    CANTIDAD_SUBESTRATOS
} Subestrato;

typedef struct
{
    TipoCelda tipo;
    int idHogar;
    int zona;
    double precio;
} Celda;

typedef struct
{
    int id;
    int idCelda;
    Subestrato subestrato;
    ClaseSocioeconomica clase;
    double ingresoMensual;
    int mesesBloqueado;
    bool satisfecho;
} Hogar;

typedef struct
{
    int ancho;
    int alto;
    int cantidadCeldas;
    int cantidadHogares;
    Celda *celdas;
    Hogar *hogares;
} Modelo;

bool crearModelo(Modelo *modelo, int ancho, int alto, int cantidadHogares);
void liberarModelo(Modelo *modelo);
int obtenerIdCelda(const Modelo *modelo, int fila, int columna);
bool obtenerCoordenadas(const Modelo *modelo, int idCelda, int *fila, int *columna);
bool definirCelda(Modelo *modelo, int idCelda, TipoCelda tipo, int zona, double precio);
bool ubicarHogar(Modelo *modelo, int idHogar, int idCelda, Subestrato subestrato,
                 double ingresoMensual);
ClaseSocioeconomica obtenerClaseSubestrato(Subestrato subestrato);
double obtenerIngresoBase(Subestrato subestrato);
int contarCeldasResidenciales(const Modelo *modelo);
int contarViviendasVacias(const Modelo *modelo);
bool validarModelo(const Modelo *modelo);

#endif
