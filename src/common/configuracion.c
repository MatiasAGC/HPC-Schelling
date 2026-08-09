#include "schelling/configuracion.h"

#include "schelling/registro.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LONGITUD_LINEA 512

static char *recortar(char *texto)
{
    char *fin;

    while (isspace((unsigned char)*texto) != 0)
    {
        texto++;
    }

    if (*texto == '\0')
    {
        return texto;
    }

    fin = texto + strlen(texto) - 1;

    while (fin > texto && isspace((unsigned char)*fin) != 0)
    {
        *fin = '\0';
        fin--;
    }

    return texto;
}

static bool convertirEntero(const char *texto, int *resultado)
{
    char *fin = NULL;
    long valor;

    errno = 0;
    valor = strtol(texto, &fin, 10);

    if (errno != 0 || fin == texto || *fin != '\0' || valor < 0 || valor > INT_MAX)
    {
        return false;
    }

    *resultado = (int)valor;
    return true;
}

static bool convertirSemilla(const char *texto, uint64_t *resultado)
{
    char *fin = NULL;
    unsigned long long valor;

    if (*texto == '-')
    {
        return false;
    }

    errno = 0;
    valor = strtoull(texto, &fin, 10);

    if (errno != 0 || fin == texto || *fin != '\0')
    {
        return false;
    }

    *resultado = (uint64_t)valor;
    return true;
}

static bool convertirReal(const char *texto, double *resultado)
{
    char *fin = NULL;
    double valor;

    errno = 0;
    valor = strtod(texto, &fin);

    if (errno != 0 || fin == texto || *fin != '\0' || !isfinite(valor))
    {
        return false;
    }

    *resultado = valor;
    return true;
}

static bool convertirBooleano(const char *texto, bool *resultado)
{
    if (strcmp(texto, "true") == 0 || strcmp(texto, "1") == 0)
    {
        *resultado = true;
        return true;
    }

    if (strcmp(texto, "false") == 0 || strcmp(texto, "0") == 0)
    {
        *resultado = false;
        return true;
    }

    return false;
}

static bool asignarValor(Configuracion *configuracion, const char *clave, const char *valor)
{
    if (strcmp(clave, "ancho") == 0)
    {
        return convertirEntero(valor, &configuracion->ancho);
    }
    if (strcmp(clave, "alto") == 0)
    {
        return convertirEntero(valor, &configuracion->alto);
    }
    if (strcmp(clave, "iteraciones") == 0)
    {
        return convertirEntero(valor, &configuracion->iteraciones);
    }
    if (strcmp(clave, "radioVecindario") == 0)
    {
        return convertirEntero(valor, &configuracion->radioVecindario);
    }
    if (strcmp(clave, "sigma") == 0)
    {
        return convertirReal(valor, &configuracion->sigma);
    }
    if (strcmp(clave, "semilla") == 0)
    {
        return convertirSemilla(valor, &configuracion->semilla);
    }
    if (strcmp(clave, "permanenciaMinima") == 0)
    {
        return convertirEntero(valor, &configuracion->permanenciaMinima);
    }
    if (strcmp(clave, "frecuenciaCheckpoint") == 0)
    {
        return convertirEntero(valor, &configuracion->frecuenciaCheckpoint);
    }
    if (strcmp(clave, "cantidadCheckpoints") == 0)
    {
        return convertirEntero(valor, &configuracion->cantidadCheckpoints);
    }
    if (strcmp(clave, "alpha0") == 0)
    {
        return convertirReal(valor, &configuracion->alpha0);
    }
    if (strcmp(clave, "beta1") == 0)
    {
        return convertirReal(valor, &configuracion->beta1);
    }
    if (strcmp(clave, "beta2") == 0)
    {
        return convertirReal(valor, &configuracion->beta2);
    }
    if (strcmp(clave, "rho") == 0)
    {
        return convertirReal(valor, &configuracion->rho);
    }
    if (strcmp(clave, "desviacionRuido") == 0)
    {
        return convertirReal(valor, &configuracion->desviacionRuido);
    }
    if (strcmp(clave, "limiteRuido") == 0)
    {
        return convertirReal(valor, &configuracion->limiteRuido);
    }
    if (strcmp(clave, "ruidoHabilitado") == 0)
    {
        return convertirBooleano(valor, &configuracion->ruidoHabilitado);
    }
    if (strcmp(clave, "fraccionFinanciada") == 0)
    {
        return convertirReal(valor, &configuracion->fraccionFinanciada);
    }
    if (strcmp(clave, "tasaAnual") == 0)
    {
        return convertirReal(valor, &configuracion->tasaAnual);
    }
    if (strcmp(clave, "plazoMeses") == 0)
    {
        return convertirEntero(valor, &configuracion->plazoMeses);
    }

    registrarError("clave de configuracion desconocida %s", clave);
    return false;
}

void iniciarConfiguracionPredeterminada(Configuracion *configuracion)
{
    configuracion->ancho = 1024;
    configuracion->alto = 640;
    configuracion->iteraciones = 120;
    configuracion->radioVecindario = 2;
    configuracion->sigma = 1.0;
    configuracion->semilla = UINT64_C(42);
    configuracion->permanenciaMinima = 12;
    configuracion->frecuenciaCheckpoint = 50;
    configuracion->cantidadCheckpoints = 2;
    configuracion->alpha0 = 6.793886203;
    configuracion->beta1 = 0.40;
    configuracion->beta2 = 0.60;
    configuracion->rho = 0.30;
    configuracion->desviacionRuido = 0.05;
    configuracion->limiteRuido = 0.15;
    configuracion->ruidoHabilitado = true;
    configuracion->fraccionFinanciada = 0.80;
    configuracion->tasaAnual = 0.06;
    configuracion->plazoMeses = 240;
}

bool cargarConfiguracion(const char *ruta, Configuracion *configuracion)
{
    FILE *archivo = fopen(ruta, "r");
    char linea[LONGITUD_LINEA];
    int numeroLinea = 0;

    if (archivo == NULL)
    {
        registrarError("no se pudo abrir la configuracion %s", ruta);
        return false;
    }

    while (fgets(linea, sizeof(linea), archivo) != NULL)
    {
        char *clave;
        const char *valor;
        char *separador;

        numeroLinea++;
        clave = recortar(linea);

        if (*clave == '\0' || *clave == '#')
        {
            continue;
        }

        separador = strchr(clave, '=');

        if (separador == NULL)
        {
            registrarError("linea %d sin separador en %s", numeroLinea, ruta);
            fclose(archivo);
            return false;
        }

        *separador = '\0';
        valor = recortar(separador + 1);
        clave = recortar(clave);

        if (*clave == '\0' || *valor == '\0' || !asignarValor(configuracion, clave, valor))
        {
            registrarError("valor invalido en linea %d de %s", numeroLinea, ruta);
            fclose(archivo);
            return false;
        }
    }

    if (ferror(archivo) != 0)
    {
        registrarError("error al leer la configuracion %s", ruta);
        fclose(archivo);
        return false;
    }

    fclose(archivo);
    return validarConfiguracion(configuracion);
}

bool validarConfiguracion(const Configuracion *configuracion)
{
    if (configuracion->ancho <= 0 || configuracion->alto <= 0)
    {
        registrarError("las dimensiones deben ser positivas");
        return false;
    }

    if (configuracion->iteraciones <= 0 || configuracion->radioVecindario <= 0)
    {
        registrarError("iteraciones y radio deben ser positivos");
        return false;
    }

    if (!isfinite(configuracion->sigma) || !isfinite(configuracion->alpha0) ||
        !isfinite(configuracion->beta1) || !isfinite(configuracion->beta2) ||
        !isfinite(configuracion->rho) || !isfinite(configuracion->desviacionRuido) ||
        !isfinite(configuracion->limiteRuido) || !isfinite(configuracion->fraccionFinanciada) ||
        !isfinite(configuracion->tasaAnual))
    {
        registrarError("los parametros reales deben ser finitos");
        return false;
    }

    if (configuracion->sigma <= 0.0)
    {
        registrarError("sigma debe ser positivo");
        return false;
    }

    if (configuracion->rho <= 0.0 || configuracion->rho > 1.0 ||
        configuracion->fraccionFinanciada < 0.0 || configuracion->fraccionFinanciada > 1.0)
    {
        registrarError("rho y fraccion financiada deben pertenecer a sus rangos");
        return false;
    }

    if (configuracion->tasaAnual < 0.0 || configuracion->plazoMeses <= 0)
    {
        registrarError("la financiacion es invalida");
        return false;
    }

    if (configuracion->desviacionRuido < 0.0 || configuracion->limiteRuido < 0.0)
    {
        registrarError("los parametros de ruido no pueden ser negativos");
        return false;
    }

    return true;
}

void mostrarConfiguracion(const Configuracion *configuracion)
{
    registrarInformacion("grilla %d x %d", configuracion->ancho, configuracion->alto);
    registrarInformacion("iteraciones %d", configuracion->iteraciones);
    registrarInformacion("vecindario radio %d sigma %.3f", configuracion->radioVecindario,
                         configuracion->sigma);
    registrarInformacion("semilla %llu", (unsigned long long)configuracion->semilla);
    registrarInformacion("checkpoint cada %d iteraciones", configuracion->frecuenciaCheckpoint);
}
