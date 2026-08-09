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
    if (strcmp(clave, "proporcionResidencial") == 0)
    {
        return convertirReal(valor, &configuracion->proporcionResidencial);
    }
    if (strcmp(clave, "proporcionOcupacion") == 0)
    {
        return convertirReal(valor, &configuracion->proporcionOcupacion);
    }
    if (strcmp(clave, "cantidadZonas") == 0)
    {
        return convertirEntero(valor, &configuracion->cantidadZonas);
    }
    if (strcmp(clave, "tamanoBloqueVacantes") == 0)
    {
        return convertirEntero(valor, &configuracion->tamanoBloqueVacantes);
    }
    if (strcmp(clave, "pesoSubestratoAMas") == 0)
    {
        return convertirReal(valor, &configuracion->pesosSubestratos[SUBESTRATO_A_MAS]);
    }
    if (strcmp(clave, "pesoSubestratoAMenos") == 0)
    {
        return convertirReal(valor, &configuracion->pesosSubestratos[SUBESTRATO_A_MENOS]);
    }
    if (strcmp(clave, "pesoSubestratoMMas") == 0)
    {
        return convertirReal(valor, &configuracion->pesosSubestratos[SUBESTRATO_M_MAS]);
    }
    if (strcmp(clave, "pesoSubestratoM") == 0)
    {
        return convertirReal(valor, &configuracion->pesosSubestratos[SUBESTRATO_M]);
    }
    if (strcmp(clave, "pesoSubestratoMMenos") == 0)
    {
        return convertirReal(valor, &configuracion->pesosSubestratos[SUBESTRATO_M_MENOS]);
    }
    if (strcmp(clave, "pesoSubestratoBMas") == 0)
    {
        return convertirReal(valor, &configuracion->pesosSubestratos[SUBESTRATO_B_MAS]);
    }
    if (strcmp(clave, "pesoSubestratoBMenos") == 0)
    {
        return convertirReal(valor, &configuracion->pesosSubestratos[SUBESTRATO_B_MENOS]);
    }
    if (strcmp(clave, "toleranciaAltaAlta") == 0)
    {
        return convertirReal(valor, &configuracion->tolerancias[CLASE_ALTA][CLASE_ALTA]);
    }
    if (strcmp(clave, "toleranciaAltaMedia") == 0)
    {
        return convertirReal(valor, &configuracion->tolerancias[CLASE_ALTA][CLASE_MEDIA]);
    }
    if (strcmp(clave, "toleranciaAltaBaja") == 0)
    {
        return convertirReal(valor, &configuracion->tolerancias[CLASE_ALTA][CLASE_BAJA]);
    }
    if (strcmp(clave, "toleranciaMediaAlta") == 0)
    {
        return convertirReal(valor, &configuracion->tolerancias[CLASE_MEDIA][CLASE_ALTA]);
    }
    if (strcmp(clave, "toleranciaMediaMedia") == 0)
    {
        return convertirReal(valor, &configuracion->tolerancias[CLASE_MEDIA][CLASE_MEDIA]);
    }
    if (strcmp(clave, "toleranciaMediaBaja") == 0)
    {
        return convertirReal(valor, &configuracion->tolerancias[CLASE_MEDIA][CLASE_BAJA]);
    }
    if (strcmp(clave, "toleranciaBajaAlta") == 0)
    {
        return convertirReal(valor, &configuracion->tolerancias[CLASE_BAJA][CLASE_ALTA]);
    }
    if (strcmp(clave, "toleranciaBajaMedia") == 0)
    {
        return convertirReal(valor, &configuracion->tolerancias[CLASE_BAJA][CLASE_MEDIA]);
    }
    if (strcmp(clave, "toleranciaBajaBaja") == 0)
    {
        return convertirReal(valor, &configuracion->tolerancias[CLASE_BAJA][CLASE_BAJA]);
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
    configuracion->proporcionResidencial = 0.885;
    configuracion->proporcionOcupacion = 0.90;
    configuracion->cantidadZonas = 8;
    configuracion->tamanoBloqueVacantes = 32;
    configuracion->pesosSubestratos[SUBESTRATO_A_MAS] = 8.6;
    configuracion->pesosSubestratos[SUBESTRATO_A_MENOS] = 17.1;
    configuracion->pesosSubestratos[SUBESTRATO_M_MAS] = 21.2;
    configuracion->pesosSubestratos[SUBESTRATO_M] = 21.8;
    configuracion->pesosSubestratos[SUBESTRATO_M_MENOS] = 16.0;
    configuracion->pesosSubestratos[SUBESTRATO_B_MAS] = 10.4;
    configuracion->pesosSubestratos[SUBESTRATO_B_MENOS] = 5.0;
    configuracion->tolerancias[CLASE_ALTA][CLASE_ALTA] = 0.70;
    configuracion->tolerancias[CLASE_ALTA][CLASE_MEDIA] = 0.30;
    configuracion->tolerancias[CLASE_ALTA][CLASE_BAJA] = 0.05;
    configuracion->tolerancias[CLASE_MEDIA][CLASE_ALTA] = 0.70;
    configuracion->tolerancias[CLASE_MEDIA][CLASE_MEDIA] = 0.50;
    configuracion->tolerancias[CLASE_MEDIA][CLASE_BAJA] = 0.40;
    configuracion->tolerancias[CLASE_BAJA][CLASE_ALTA] = 0.80;
    configuracion->tolerancias[CLASE_BAJA][CLASE_MEDIA] = 0.90;
    configuracion->tolerancias[CLASE_BAJA][CLASE_BAJA] = 0.02;
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
    if (configuracion->ancho <= 0 || configuracion->alto <= 0 ||
        configuracion->ancho > INT_MAX / configuracion->alto)
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
        !isfinite(configuracion->tasaAnual) || !isfinite(configuracion->proporcionResidencial) ||
        !isfinite(configuracion->proporcionOcupacion))
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

    if (configuracion->frecuenciaCheckpoint < 0 || configuracion->cantidadCheckpoints <= 0 ||
        configuracion->permanenciaMinima < 0)
    {
        registrarError("la configuracion de permanencia o checkpoints es invalida");
        return false;
    }

    if (configuracion->proporcionResidencial < 0.0 || configuracion->proporcionResidencial > 1.0 ||
        configuracion->proporcionOcupacion < 0.0 || configuracion->proporcionOcupacion > 1.0 ||
        configuracion->cantidadZonas <= 0 || configuracion->tamanoBloqueVacantes <= 0)
    {
        registrarError("las proporciones de generacion o la cantidad de zonas son invalidas");
        return false;
    }

    double sumaPesosSubestratos = 0.0;

    for (int subestrato = 0; subestrato < CANTIDAD_SUBESTRATOS; subestrato++)
    {
        double peso = configuracion->pesosSubestratos[subestrato];

        if (!isfinite(peso) || peso < 0.0)
        {
            registrarError("los pesos de subestratos no pueden ser negativos");
            return false;
        }

        sumaPesosSubestratos += peso;
    }

    if (sumaPesosSubestratos <= 0.0 || !isfinite(sumaPesosSubestratos))
    {
        registrarError("al menos un subestrato debe tener peso positivo");
        return false;
    }

    for (int claseHogar = 0; claseHogar < CANTIDAD_CLASES; claseHogar++)
    {
        for (int claseVecina = 0; claseVecina < CANTIDAD_CLASES; claseVecina++)
        {
            double tolerancia = configuracion->tolerancias[claseHogar][claseVecina];

            if (!isfinite(tolerancia) || tolerancia < 0.0 || tolerancia > 1.0)
            {
                registrarError("las tolerancias deben pertenecer al intervalo de cero a uno");
                return false;
            }
        }
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
