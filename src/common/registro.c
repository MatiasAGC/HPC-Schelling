#include "schelling/registro.h"

#include <stdarg.h>
#include <stdio.h>

static void registrar(FILE *salida, const char *nivel, const char *formato, va_list argumentos)
{
    fprintf(salida, "[%s] ", nivel);
    vfprintf(salida, formato, argumentos);
    fputc('\n', salida);
}

void registrarInformacion(const char *formato, ...)
{
    va_list argumentos;

    va_start(argumentos, formato);
    registrar(stdout, "info", formato, argumentos);
    va_end(argumentos);
}

void registrarError(const char *formato, ...)
{
    va_list argumentos;

    va_start(argumentos, formato);
    registrar(stderr, "error", formato, argumentos);
    va_end(argumentos);
}
