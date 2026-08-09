#include "schelling/argumentos.h"

#include "schelling/registro.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool copiarRuta(char *destino, const char *origen)
{
    size_t longitud = strlen(origen);

    if (longitud >= LONGITUD_RUTA)
    {
        registrarError("la ruta supera el maximo de %d caracteres", LONGITUD_RUTA - 1);
        return false;
    }

    memcpy(destino, origen, longitud + 1);
    return true;
}

static bool leerEntero(const char *texto, int *valor)
{
    char *fin = NULL;
    long resultado;

    errno = 0;
    resultado = strtol(texto, &fin, 10);

    if (errno != 0 || fin == texto || *fin != '\0' || resultado < 0 || resultado > INT_MAX)
    {
        return false;
    }

    *valor = (int)resultado;
    return true;
}

static bool leerSemilla(const char *texto, uint64_t *valor)
{
    char *fin = NULL;
    unsigned long long resultado;

    if (*texto == '-')
    {
        return false;
    }

    errno = 0;
    resultado = strtoull(texto, &fin, 10);

    if (errno != 0 || fin == texto || *fin != '\0')
    {
        return false;
    }

    *valor = (uint64_t)resultado;
    return true;
}

static bool requiereValor(int indice, int argc, const char *opcion)
{
    if (indice + 1 < argc)
    {
        return true;
    }

    registrarError("falta el valor de %s", opcion);
    return false;
}

void iniciarOpcionesPrograma(OpcionesPrograma *opciones)
{
    memset(opciones, 0, sizeof(*opciones));
    copiarRuta(opciones->rutaConfiguracion, "config/base.conf");
    copiarRuta(opciones->rutaSalida, "output");
}

bool leerArgumentos(int argc, char **argv, OpcionesPrograma *opciones)
{
    for (int indice = 1; indice < argc; indice++)
    {
        const char *argumento = argv[indice];

        if (strcmp(argumento, "--help") == 0)
        {
            opciones->mostrarAyuda = true;
        }
        else if (strcmp(argumento, "--version") == 0)
        {
            opciones->mostrarVersion = true;
        }
        else if (strcmp(argumento, "--validate") == 0)
        {
            opciones->validar = true;
        }
        else if (strcmp(argumento, "--config") == 0)
        {
            if (!requiereValor(indice, argc, argumento))
            {
                return false;
            }

            indice++;

            if (!copiarRuta(opciones->rutaConfiguracion, argv[indice]))
            {
                return false;
            }
        }
        else if (strcmp(argumento, "--input") == 0)
        {
            if (!requiereValor(indice, argc, argumento))
            {
                return false;
            }

            indice++;

            if (!copiarRuta(opciones->rutaEntrada, argv[indice]))
            {
                return false;
            }
        }
        else if (strcmp(argumento, "--output") == 0)
        {
            if (!requiereValor(indice, argc, argumento))
            {
                return false;
            }

            indice++;

            if (!copiarRuta(opciones->rutaSalida, argv[indice]))
            {
                return false;
            }
        }
        else if (strcmp(argumento, "--restart") == 0)
        {
            if (!requiereValor(indice, argc, argumento))
            {
                return false;
            }

            indice++;

            if (!copiarRuta(opciones->rutaReinicio, argv[indice]))
            {
                return false;
            }
        }
        else if (strcmp(argumento, "--seed") == 0)
        {
            if (!requiereValor(indice, argc, argumento) ||
                !leerSemilla(argv[++indice], &opciones->semilla))
            {
                registrarError("semilla invalida");
                return false;
            }

            opciones->sobrescribirSemilla = true;
        }
        else if (strcmp(argumento, "--iterations") == 0)
        {
            if (!requiereValor(indice, argc, argumento) ||
                !leerEntero(argv[++indice], &opciones->iteraciones))
            {
                registrarError("cantidad de iteraciones invalida");
                return false;
            }

            opciones->sobrescribirIteraciones = true;
        }
        else if (strcmp(argumento, "--checkpoint-every") == 0)
        {
            if (!requiereValor(indice, argc, argumento) ||
                !leerEntero(argv[++indice], &opciones->frecuenciaCheckpoint))
            {
                registrarError("frecuencia de checkpoint invalida");
                return false;
            }

            opciones->sobrescribirCheckpoint = true;
        }
        else
        {
            registrarError("opcion desconocida %s", argumento);
            return false;
        }
    }

    return true;
}

void mostrarUso(const char *nombrePrograma)
{
    printf("uso: %s [opciones]\n", nombrePrograma);
    printf("  --config ruta\n");
    printf("  --seed numero\n");
    printf("  --iterations numero\n");
    printf("  --input ruta\n");
    printf("  --output ruta\n");
    printf("  --checkpoint-every numero\n");
    printf("  --restart ruta\n");
    printf("  --validate\n");
    printf("  --help\n");
    printf("  --version\n");
}
