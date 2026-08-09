#ifndef SCHELLING_ARGUMENTOS_H
#define SCHELLING_ARGUMENTOS_H

#include <stdbool.h>
#include <stdint.h>

#define LONGITUD_RUTA 512

typedef struct
{
    char rutaConfiguracion[LONGITUD_RUTA];
    char rutaEntrada[LONGITUD_RUTA];
    char rutaSalida[LONGITUD_RUTA];
    char rutaReinicio[LONGITUD_RUTA];
    uint64_t semilla;
    int iteraciones;
    int frecuenciaCheckpoint;
    bool sobrescribirSemilla;
    bool sobrescribirIteraciones;
    bool sobrescribirCheckpoint;
    bool validar;
    bool mostrarAyuda;
    bool mostrarVersion;
} OpcionesPrograma;

void iniciarOpcionesPrograma(OpcionesPrograma *opciones);
bool leerArgumentos(int argc, char **argv, OpcionesPrograma *opciones);
void mostrarUso(const char *nombrePrograma);

#endif
