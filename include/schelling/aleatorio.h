#ifndef SCHELLING_ALEATORIO_H
#define SCHELLING_ALEATORIO_H

#include <stdint.h>

typedef enum
{
    PROPOSITO_TIPO_CELDA = 1,
    PROPOSITO_OCUPACION = 2,
    PROPOSITO_SUBESTRATO = 3,
    PROPOSITO_RUIDO_PRECIO = 4,
    PROPOSITO_DESEMPATE = 5
} PropositoAleatorio;

uint64_t mezclar64(uint64_t valor);
uint64_t generarBitsAleatorios(uint64_t semilla, uint64_t iteracion, uint64_t idEntidad,
                               PropositoAleatorio proposito, uint64_t numeroMuestra);
double generarUniforme(uint64_t semilla, uint64_t iteracion, uint64_t idEntidad,
                       PropositoAleatorio proposito, uint64_t numeroMuestra);
double generarNormal(uint64_t semilla, uint64_t iteracion, uint64_t idEntidad,
                     PropositoAleatorio proposito, uint64_t numeroMuestra);

#endif
