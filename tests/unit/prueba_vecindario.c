#include "schelling/configuracion.h"
#include "schelling/modelo.h"
#include "schelling/vecindario.h"

#include <math.h>
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

static void verificarReal(double esperado, double obtenido, const char *mensaje)
{
    if (fabs(esperado - obtenido) > 1e-12)
    {
        fprintf(stderr, "%s esperado %.12f obtenido %.12f\n", mensaje, esperado, obtenido);
        cantidadFallos++;
    }
}

static void pesosGaussianosSonCorrectos(void)
{
    Vecindario vecindario = {0};

    verificar(crearVecindario(&vecindario, 1, 1.0), "no se pudo crear el vecindario");
    verificar(vecindario.cantidadDesplazamientos == 8,
              "un vecindario de radio uno debe tener ocho desplazamientos");
    verificarReal(exp(-1.0), vecindario.desplazamientos[0].peso, "el peso diagonal es incorrecto");
    verificarReal(exp(-0.5), vecindario.desplazamientos[1].peso, "el peso vertical es incorrecto");
    liberarVecindario(&vecindario);
}

static void proporcionesExcluyenCeldasSinHogar(void)
{
    Modelo modelo = {0};
    Vecindario vecindario = {0};
    double proporciones[CANTIDAD_CLASES];
    bool aislado = true;

    verificar(crearModelo(&modelo, 3, 3, 2), "no se pudo crear el modelo de proporciones");
    verificar(crearVecindario(&vecindario, 1, 1.0), "no se pudo crear el vecindario de prueba");
    verificar(ubicarHogar(&modelo, 0, 1, SUBESTRATO_A_MAS, 122.5),
              "no se pudo ubicar el hogar alto");
    verificar(ubicarHogar(&modelo, 1, 7, SUBESTRATO_M, 33.2), "no se pudo ubicar el hogar medio");
    verificar(definirCelda(&modelo, 3, CELDA_NO_RESIDENCIAL, 0, 0.0),
              "no se pudo definir una celda no residencial");
    verificar(calcularProporciones(&modelo, &vecindario, 4, proporciones, &aislado),
              "no se pudieron calcular las proporciones");
    verificar(!aislado, "el centro no deberia estar aislado");
    verificarReal(0.5, proporciones[CLASE_ALTA], "la proporcion alta es incorrecta");
    verificarReal(0.5, proporciones[CLASE_MEDIA], "la proporcion media es incorrecta");
    verificarReal(0.0, proporciones[CLASE_BAJA], "la proporcion baja es incorrecta");
    liberarVecindario(&vecindario);
    liberarModelo(&modelo);
}

static void limitesExactosSonInclusivos(void)
{
    Configuracion configuracion;
    double alta[CANTIDAD_CLASES] = {0.70, 0.25, 0.05};
    double media[CANTIDAD_CLASES] = {0.10, 0.50, 0.40};
    double baja[CANTIDAD_CLASES] = {0.08, 0.90, 0.02};

    iniciarConfiguracionPredeterminada(&configuracion);
    verificar(cumpleTolerancias(CLASE_ALTA, alta, &configuracion),
              "la clase alta deberia aceptar los limites exactos");
    verificar(cumpleTolerancias(CLASE_MEDIA, media, &configuracion),
              "la clase media deberia aceptar los limites exactos");
    verificar(cumpleTolerancias(CLASE_BAJA, baja, &configuracion),
              "la clase baja deberia aceptar los limites exactos");

    alta[CLASE_BAJA] = 0.051;
    media[CLASE_MEDIA] = 0.499;
    baja[CLASE_BAJA] = 0.019;
    verificar(!cumpleTolerancias(CLASE_ALTA, alta, &configuracion),
              "la clase alta deberia rechazar el exceso de clase baja");
    verificar(!cumpleTolerancias(CLASE_MEDIA, media, &configuracion),
              "la clase media deberia rechazar una proporcion media insuficiente");
    verificar(!cumpleTolerancias(CLASE_BAJA, baja, &configuracion),
              "la clase baja deberia rechazar una proporcion baja insuficiente");
}

static void aislamientoProduceSatisfaccion(void)
{
    Modelo modelo = {0};
    Vecindario vecindario = {0};
    Configuracion configuracion;
    bool aislado = false;

    iniciarConfiguracionPredeterminada(&configuracion);
    verificar(crearModelo(&modelo, 3, 3, 0), "no se pudo crear el modelo aislado");
    verificar(crearVecindario(&vecindario, 1, 1.0), "no se pudo crear el vecindario aislado");
    verificar(evaluarSatisfaccion(&modelo, &vecindario, 4, CLASE_MEDIA, &configuracion, &aislado),
              "un hogar aislado deberia considerarse satisfecho");
    verificar(aislado, "el caso deberia marcarse como aislado");
    liberarVecindario(&vecindario);
    liberarModelo(&modelo);
}

int main(void)
{
    pesosGaussianosSonCorrectos();
    proporcionesExcluyenCeldasSinHogar();
    limitesExactosSonInclusivos();
    aislamientoProduceSatisfaccion();

    if (cantidadFallos != 0)
    {
        fprintf(stderr, "fallaron %d verificaciones\n", cantidadFallos);
        return 1;
    }

    printf("pruebas de vecindario correctas\n");
    return 0;
}
