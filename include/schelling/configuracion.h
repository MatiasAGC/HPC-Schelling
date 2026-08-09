#ifndef SCHELLING_CONFIGURACION_H
#define SCHELLING_CONFIGURACION_H

#include "schelling/modelo.h"

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    int ancho;
    int alto;
    int iteraciones;
    int radioVecindario;
    int permanenciaMinima;
    int frecuenciaCheckpoint;
    int cantidadCheckpoints;
    uint64_t semilla;
    double sigma;
    double alpha0;
    double beta1;
    double beta2;
    double rho;
    double desviacionRuido;
    double limiteRuido;
    double fraccionFinanciada;
    double tasaAnual;
    int plazoMeses;
    bool ruidoHabilitado;
    double proporcionResidencial;
    double proporcionOcupacion;
    int cantidadZonas;
    int tamanoBloqueVacantes;
    double pesosSubestratos[CANTIDAD_SUBESTRATOS];
    double tolerancias[CANTIDAD_CLASES][CANTIDAD_CLASES];
} Configuracion;

void iniciarConfiguracionPredeterminada(Configuracion *configuracion);
bool cargarConfiguracion(const char *ruta, Configuracion *configuracion);
bool validarConfiguracion(const Configuracion *configuracion);
void mostrarConfiguracion(const Configuracion *configuracion);

#endif
