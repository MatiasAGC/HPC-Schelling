#include "schelling/configuracion.h"
#include "schelling/generador.h"
#include "schelling/hash.h"
#include "schelling/modelo.h"

#include <stdbool.h>
#include <stdio.h>

int main(void)
{
    Configuracion configuracion;
    Modelo primero = {0};
    Modelo segundo = {0};
    uint64_t hashOriginal;

    iniciarConfiguracionPredeterminada(&configuracion);
    configuracion.ancho = 8;
    configuracion.alto = 6;

    if (!generarModeloSintetico(&configuracion, &primero) ||
        !generarModeloSintetico(&configuracion, &segundo))
    {
        return 1;
    }

    hashOriginal = calcularHashModelo(&primero);

    if (hashOriginal != calcularHashModelo(&segundo))
    {
        fprintf(stderr, "dos estados iguales deben tener el mismo hash\n");
        return 1;
    }

    segundo.celdas[0].precio += 1.0;

    if (hashOriginal == calcularHashModelo(&segundo))
    {
        fprintf(stderr, "un cambio de precio debe modificar el hash\n");
        return 1;
    }

    liberarModelo(&primero);
    liberarModelo(&segundo);
    printf("pruebas de hash correctas\n");
    return 0;
}
