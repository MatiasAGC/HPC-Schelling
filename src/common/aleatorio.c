#include "schelling/aleatorio.h"

#include <math.h>

uint64_t mezclar64(uint64_t valor)
{
    valor += UINT64_C(0x9e3779b97f4a7c15);
    valor = (valor ^ (valor >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
    valor = (valor ^ (valor >> 27)) * UINT64_C(0x94d049bb133111eb);
    return valor ^ (valor >> 31);
}

uint64_t generarBitsAleatorios(uint64_t semilla, uint64_t iteracion, uint64_t idEntidad,
                               PropositoAleatorio proposito, uint64_t numeroMuestra)
{
    uint64_t clave = mezclar64(semilla);

    clave ^= mezclar64(iteracion + UINT64_C(0x243f6a8885a308d3));
    clave ^= mezclar64(idEntidad + UINT64_C(0x13198a2e03707344));
    clave ^= mezclar64((uint64_t)proposito + UINT64_C(0xa4093822299f31d0));
    clave ^= mezclar64(numeroMuestra + UINT64_C(0x082efa98ec4e6c89));
    return mezclar64(clave);
}

double generarUniforme(uint64_t semilla, uint64_t iteracion, uint64_t idEntidad,
                       PropositoAleatorio proposito, uint64_t numeroMuestra)
{
    uint64_t bits = generarBitsAleatorios(semilla, iteracion, idEntidad, proposito, numeroMuestra);
    uint64_t mantisa = bits >> 11;

    return ((double)mantisa + 0.5) * (1.0 / 9007199254740992.0);
}

double generarNormal(uint64_t semilla, uint64_t iteracion, uint64_t idEntidad,
                     PropositoAleatorio proposito, uint64_t numeroMuestra)
{
    double primero = generarUniforme(semilla, iteracion, idEntidad, proposito, numeroMuestra * 2);
    double segundo =
        generarUniforme(semilla, iteracion, idEntidad, proposito, numeroMuestra * 2 + 1);

    return sqrt(-2.0 * log(primero)) * cos(2.0 * acos(-1.0) * segundo);
}
