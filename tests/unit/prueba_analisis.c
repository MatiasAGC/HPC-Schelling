#include "schelling/analisis.h"
#include "schelling/modelo.h"
#include "schelling/vecindario.h"

#include <math.h>
#include <stdio.h>

int main(void)
{
    Modelo modelo = {0};
    Vecindario vecindario = {0};
    MetricasSegregacion metricas;

    if (!crearModelo(&modelo, 3, 1, 3) || !ubicarHogar(&modelo, 0, 0, SUBESTRATO_A_MAS, 1.0) ||
        !ubicarHogar(&modelo, 1, 1, SUBESTRATO_A_MENOS, 1.0) ||
        !ubicarHogar(&modelo, 2, 2, SUBESTRATO_B_MAS, 1.0) ||
        !crearVecindario(&vecindario, 1, 1.0) ||
        !calcularMetricasSegregacion(&modelo, &vecindario, &metricas))
    {
        return 1;
    }

    if (metricas.hogaresConVecinos != 3 || metricas.hogaresAislados != 0 ||
        fabs(metricas.proporcionMismaClase - 0.5) > 1e-9)
    {
        fprintf(stderr, "metrica de segregacion incorrecta\n");
        return 1;
    }

    liberarVecindario(&vecindario);
    liberarModelo(&modelo);
    printf("pruebas de analisis correctas\n");
    return 0;
}
