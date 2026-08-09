#include "schelling/aleatorio.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>

static int cantidadFallos = 0;

static void verificar(bool condicion, const char *mensaje)
{
    if (!condicion)
    {
        fprintf(stderr, "%s\n", mensaje);
        cantidadFallos++;
    }
}

static void mezclaTieneVectorFijo(void)
{
    uint64_t obtenido = mezclar64(UINT64_C(0));
    uint64_t esperado = UINT64_C(0xe220a8397b1dcdaf);
    uint64_t segundoObtenido = mezclar64(UINT64_C(1));
    uint64_t segundoEsperado = UINT64_C(0x910a2dec89025cc1);

    if (obtenido != esperado)
    {
        fprintf(stderr, "mezcla esperada %016" PRIx64 " obtenida %016" PRIx64 "\n", esperado,
                obtenido);
        cantidadFallos++;
    }

    if (segundoObtenido != segundoEsperado)
    {
        fprintf(stderr, "segunda mezcla esperada %016" PRIx64 " obtenida %016" PRIx64 "\n",
                segundoEsperado, segundoObtenido);
        cantidadFallos++;
    }
}

static void clavesSonDeterministas(void)
{
    uint64_t primero = generarBitsAleatorios(42, 3, 17, PROPOSITO_OCUPACION, 0);
    uint64_t repetido = generarBitsAleatorios(42, 3, 17, PROPOSITO_OCUPACION, 0);
    uint64_t otroProposito = generarBitsAleatorios(42, 3, 17, PROPOSITO_SUBESTRATO, 0);
    uint64_t otraEntidad = generarBitsAleatorios(42, 3, 18, PROPOSITO_OCUPACION, 0);

    verificar(primero == repetido, "la misma clave deberia producir los mismos bits");
    verificar(primero != otroProposito, "el proposito debe separar secuencias");
    verificar(primero != otraEntidad, "la entidad debe separar secuencias");
}

static void uniformesPertenecenAlIntervaloAbierto(void)
{
    for (uint64_t muestra = 0; muestra < 1000; muestra++)
    {
        double valor = generarUniforme(7, 0, 9, PROPOSITO_DESEMPATE, muestra);

        verificar(valor > 0.0 && valor < 1.0, "un uniforme debe pertenecer al intervalo abierto");
    }
}

int main(void)
{
    mezclaTieneVectorFijo();
    clavesSonDeterministas();
    uniformesPertenecenAlIntervaloAbierto();

    if (cantidadFallos != 0)
    {
        fprintf(stderr, "fallaron %d verificaciones\n", cantidadFallos);
        return 1;
    }

    printf("pruebas aleatorias correctas\n");
    return 0;
}
