#include "schelling/checkpoint.h"
#include "schelling/configuracion.h"
#include "schelling/generador.h"
#include "schelling/hash.h"
#include "schelling/modelo.h"
#include "schelling/simulacion.h"
#include "schelling/vecindario.h"

#include <stdint.h>
#include <stdio.h>

#define RUTA_PRUEBA "/tmp/schelling_checkpoint_prueba.bin"

int main(void)
{
    Configuracion configuracion;
    Modelo continuo = {0};
    Modelo interrumpido = {0};
    Modelo restaurado = {0};
    Modelo incompatible = {0};
    Vecindario vecindario = {0};
    MetricasIteracion metricas;
    uint64_t proximaIteracion = 0;
    int resultado = 1;

    iniciarConfiguracionPredeterminada(&configuracion);
    configuracion.ancho = 8;
    configuracion.alto = 6;
    configuracion.ruidoHabilitado = false;
    configuracion.permanenciaMinima = 1;

    if (!generarModeloSintetico(&configuracion, &continuo) ||
        !generarModeloSintetico(&configuracion, &interrumpido) ||
        !crearVecindario(&vecindario, configuracion.radioVecindario, configuracion.sigma) ||
        !ejecutarIteracion(&continuo, &vecindario, &configuracion, 0, &metricas) ||
        !ejecutarIteracion(&interrumpido, &vecindario, &configuracion, 0, &metricas) ||
        !guardarCheckpoint(RUTA_PRUEBA, &interrumpido, &configuracion, 1))
    {
        goto finalizar;
    }

    liberarModelo(&interrumpido);

    configuracion.sigma += 0.1;

    if (cargarCheckpoint(RUTA_PRUEBA, &incompatible, &configuracion, &proximaIteracion))
    {
        fprintf(stderr, "un checkpoint incompatible deberia rechazarse\n");
        goto finalizar;
    }

    configuracion.sigma -= 0.1;

    if (!cargarCheckpoint(RUTA_PRUEBA, &restaurado, &configuracion, &proximaIteracion) ||
        proximaIteracion != 1 ||
        !ejecutarIteracion(&continuo, &vecindario, &configuracion, 1, &metricas) ||
        !ejecutarIteracion(&restaurado, &vecindario, &configuracion, 1, &metricas) ||
        calcularHashModelo(&continuo) != calcularHashModelo(&restaurado))
    {
        fprintf(stderr, "el reinicio no coincide con la ejecucion continua\n");
        goto finalizar;
    }

    FILE *archivo = fopen(RUTA_PRUEBA, "ab");

    if (archivo == NULL)
    {
        goto finalizar;
    }

    int escritura = fputc(0, archivo);
    int cierre = fclose(archivo);

    if (escritura == EOF || cierre != 0)
    {
        goto finalizar;
    }

    if (cargarCheckpoint(RUTA_PRUEBA, &incompatible, &configuracion, &proximaIteracion))
    {
        fprintf(stderr, "un checkpoint alterado deberia rechazarse\n");
        goto finalizar;
    }

    resultado = 0;
    printf("pruebas de checkpoint correctas\n");

finalizar:
    remove(RUTA_PRUEBA);
    liberarVecindario(&vecindario);
    liberarModelo(&continuo);
    liberarModelo(&interrumpido);
    liberarModelo(&incompatible);
    liberarModelo(&restaurado);
    return resultado;
}
