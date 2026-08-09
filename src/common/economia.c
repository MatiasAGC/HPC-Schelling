#include "schelling/economia.h"

#include "schelling/aleatorio.h"

#include <math.h>
#include <stddef.h>

static double limitar(double valor, double minimo, double maximo)
{
    if (valor < minimo)
    {
        return minimo;
    }
    if (valor > maximo)
    {
        return maximo;
    }
    return valor;
}

double calcularCuotaMensual(double precio, const Configuracion *configuracion)
{
    double capital;
    double tasaMensual;

    if (configuracion == NULL || !isfinite(precio) || precio < 0.0)
    {
        return NAN;
    }

    capital = configuracion->fraccionFinanciada * precio;
    tasaMensual = configuracion->tasaAnual / 12.0;

    if (tasaMensual == 0.0)
    {
        return capital / configuracion->plazoMeses;
    }

    double factor = pow(1.0 + tasaMensual, configuracion->plazoMeses);
    return capital * tasaMensual * factor / (factor - 1.0);
}

bool esViviendaAccesible(double precio, const Hogar *hogar, const Configuracion *configuracion)
{
    double cuota = calcularCuotaMensual(precio, configuracion);

    return hogar != NULL && isfinite(cuota) && cuota <= configuracion->rho * hogar->ingresoMensual;
}

bool calcularEntornoEconomico(const Modelo *modelo, const Vecindario *vecindario, int idCelda,
                              double *demanda, double *poderAdquisitivo)
{
    int fila;
    int columna;
    double pesoResidencial = 0.0;
    double pesoOcupado = 0.0;
    double ingresoPonderado = 0.0;

    if (modelo == NULL || vecindario == NULL || demanda == NULL || poderAdquisitivo == NULL ||
        !obtenerCoordenadas(modelo, idCelda, &fila, &columna))
    {
        return false;
    }

    for (int indice = 0; indice < vecindario->cantidadDesplazamientos; indice++)
    {
        const DesplazamientoVecino *desplazamiento = &vecindario->desplazamientos[indice];
        int idVecina = obtenerIdCelda(modelo, fila + desplazamiento->deltaFila,
                                      columna + desplazamiento->deltaColumna);

        if (idVecina != ID_INVALIDO && modelo->celdas[idVecina].tipo == CELDA_RESIDENCIAL)
        {
            int idHogar = modelo->celdas[idVecina].idHogar;
            pesoResidencial += desplazamiento->peso;

            if (idHogar != ID_INVALIDO)
            {
                pesoOcupado += desplazamiento->peso;
                ingresoPonderado += desplazamiento->peso * modelo->hogares[idHogar].ingresoMensual;
            }
        }
    }

    *demanda = pesoResidencial == 0.0 ? 0.0 : pesoOcupado / pesoResidencial;

    if (pesoOcupado == 0.0)
    {
        *poderAdquisitivo = 0.0;
    }
    else
    {
        double ingresoMedio = ingresoPonderado / pesoOcupado;
        double minimo = obtenerIngresoBase(SUBESTRATO_B_MENOS);
        double maximo = obtenerIngresoBase(SUBESTRATO_A_MAS);
        *poderAdquisitivo = limitar((ingresoMedio - minimo) / (maximo - minimo), 0.0, 1.0);
    }

    return true;
}

bool actualizarPrecioVacio(Modelo *modelo, const Vecindario *vecindario,
                           const Configuracion *configuracion, uint64_t iteracion, int idCelda)
{
    if (modelo == NULL || vecindario == NULL || configuracion == NULL || idCelda < 0 ||
        idCelda >= modelo->cantidadCeldas)
    {
        return false;
    }

    Celda *celda = &modelo->celdas[idCelda];

    if (celda->tipo != CELDA_RESIDENCIAL || celda->idHogar != ID_INVALIDO)
    {
        return true;
    }

    double demanda;
    double poderAdquisitivo;
    double ruido = 0.0;

    if (!calcularEntornoEconomico(modelo, vecindario, idCelda, &demanda, &poderAdquisitivo))
    {
        return false;
    }

    if (configuracion->ruidoHabilitado)
    {
        ruido = generarNormal(configuracion->semilla, iteracion, (uint64_t)idCelda,
                              PROPOSITO_RUIDO_PRECIO, 0) *
                configuracion->desviacionRuido;
        ruido = limitar(ruido, -configuracion->limiteRuido, configuracion->limiteRuido);
    }

    celda->precio = exp(configuracion->alpha0 + configuracion->beta1 * demanda +
                        configuracion->beta2 * poderAdquisitivo + ruido);
    return isfinite(celda->precio);
}

bool actualizarPreciosVacios(Modelo *modelo, const Vecindario *vecindario,
                             const Configuracion *configuracion, uint64_t iteracion)
{
    if (modelo == NULL)
    {
        return false;
    }

    for (int idCelda = 0; idCelda < modelo->cantidadCeldas; idCelda++)
    {
        if (!actualizarPrecioVacio(modelo, vecindario, configuracion, iteracion, idCelda))
        {
            return false;
        }
    }

    return true;
}
