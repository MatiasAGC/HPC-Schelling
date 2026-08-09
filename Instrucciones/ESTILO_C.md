# Convenciones de código C

Estado: versión 1 aprobada para iniciar la implementación  
Alcance: código C, headers, pruebas y ejemplos del proyecto

## 1. Objetivo

El código debe ser sencillo de leer, explicar y defender ante un docente. Se priorizan claridad, nombres significativos y flujo directo por encima de abstracciones sofisticadas o reducciones artificiales de líneas.

Una persona que conoce C y el modelo debe poder seguir cada función sin necesitar comentarios que traduzcan el código.

## 2. Formato general

Se usará estilo Allman: cada llave de apertura se coloca en una línea nueva, tanto en funciones como en estructuras de control.

```c
int calcularCantidadVecinos(const Grilla *grilla, int fila, int columna)
{
    int cantidadVecinos = 0;

    if (grilla == NULL)
    {
        return 0;
    }

    for (int desplazamientoFila = -1; desplazamientoFila <= 1; desplazamientoFila++)
    {
        for (int desplazamientoColumna = -1;
             desplazamientoColumna <= 1;
             desplazamientoColumna++)
        {
            cantidadVecinos += esVecinoOcupado(grilla,
                                               fila + desplazamientoFila,
                                               columna + desplazamientoColumna);
        }
    }

    return cantidadVecinos;
}
```

Reglas de formato:

- indentación de cuatro espacios y nunca tabuladores en archivos C;
- una sentencia por línea;
- un espacio después de `if`, `for`, `while` y `switch`;
- ningún espacio entre el nombre de una función y `(`;
- operadores binarios rodeados por espacios;
- líneas de hasta 100 caracteres cuando sea razonable;
- una línea en blanco entre bloques lógicos dentro de una función;
- una línea en blanco entre funciones;
- sin espacios al final de línea;
- siempre usar llaves en `if`, `else`, `for`, `while` y `do while`, incluso cuando el cuerpo tenga una sola sentencia y sin excepciones;
- no colocar varias declaraciones distintas en una misma línea.

```c
if (hogar->mesesBloqueado > 0)
{
    return false;
}
```

No se admite:

```c
if (hogar->mesesBloqueado > 0) return false;
```

Tampoco se admite omitirlas separando la sentencia en otra línea:

```c
if (hogar->mesesBloqueado > 0)
    return false;
```

La forma correcta siempre es:

```c
if (hogar->mesesBloqueado > 0)
{
    return false;
}
```

La misma regla se aplica a ciclos y ramas `else`:

```c
for (size_t indice = 0; indice < cantidadHogares; indice++)
{
    actualizarHogar(&hogares[indice]);
}

if (mudanzaAceptada)
{
    aplicarMudanza(solicitud);
}
else
{
    registrarRechazo(solicitud);
}
```

## 3. Nombres

Las variables y funciones usan `lowerCamelCase`, como `esteEsUnEjemplo`.

```c
int cantidadVecinosOcupados;
double ingresoPonderado;
bool puedeMudarseElHogar(const Hogar *hogar);
```

Los nombres deben ser mnemotécnicos y expresar el concepto del modelo. No se usarán abreviaturas inventadas para ahorrar caracteres.

Preferir:

```c
int cantidadHogares;
double cuotaMensual;
int idCeldaDestino;
```

Evitar:

```c
int cant;
double valor;
int dato;
```

Excepciones aceptadas:

- `i`, `j` o `k` en ciclos matemáticos muy cortos;
- `x` e `y` para coordenadas cuando su significado sea evidente;
- `rank` y `size` en código MPI;
- nombres matemáticos establecidos en la especificación, como `sigma`, `alpha0`, `beta1` y `rho`.

Convenciones por categoría:

- funciones y variables: `lowerCamelCase`;
- campos de estructuras: `lowerCamelCase`;
- parámetros: `lowerCamelCase`;
- alias de tipos mediante `typedef`: `UpperCamelCase`, por ejemplo `EstadoSimulacion`;
- valores de `enum`: prefijo descriptivo y `lowerCamelCase`, por ejemplo `tipoCeldaResidencial`;
- constantes con `const`: `lowerCamelCase`;
- macros e include guards, únicamente cuando sean necesarios: `UPPER_SNAKE_CASE`;
- archivos: nombres descriptivos en minúsculas, por ejemplo `simulacion.c` y `simulacion.h`.

Los nombres propios del proyecto se escriben en español y sin tildes ni `ñ`. Por ejemplo, `calcularSatisfaccion`, `cantidadHogares` y `mesesBloqueado`. Se mantienen en inglés solamente identificadores impuestos por bibliotecas, estándares o formatos externos, como `MPI_COMM_WORLD`, `size_t` o una clave de entrada ya definida. No se mezclan palabras de ambos idiomas en un mismo identificador.

No se codifica el tipo en el nombre. Se evita notación húngara como `pGrilla`, `iCantidad` o `dPrecio`.

Las funciones usan verbos que expresen su acción:

```c
calcularSatisfaccion();
buscarDestinoMasCercano();
aplicarMudanzasAceptadas();
escribirCheckpoint();
```

Los booleanos deben poder leerse como una condición:

```c
bool estaOcupada;
bool tieneDestinoAccesible;
bool puedeMudarse;
```

## 4. Funciones

Cada función debe tener una responsabilidad clara y un nombre que la describa. Se evitarán funciones extensas que mezclen validación, cálculo, comunicación y escritura de archivos.

No se fija un límite rígido de líneas. Si una función requiere desplazarse mucho para entenderla, contiene varios niveles de abstracción o necesita comentarios para separar tareas, debe evaluarse su división.

Reglas:

- validar argumentos al comienzo;
- usar retornos tempranos para errores y casos simples;
- mantener el recorrido principal con poca indentación;
- evitar más de tres niveles de anidamiento cuando pueda simplificarse;
- no usar parámetros booleanos que cambien por completo el comportamiento de una función;
- declarar funciones internas como `static`;
- usar `const` en punteros y parámetros que no se modifican;
- evitar efectos secundarios ocultos;
- no realizar asignación de memoria dentro de una función sin dejar clara la propiedad del resultado;
- preferir funciones pequeñas del dominio antes que helpers genéricos difíciles de interpretar.

```c
bool puedeMudarse(const Hogar *hogar)
{
    if (hogar == NULL)
    {
        return false;
    }

    if (hogar->mesesBloqueado > 0)
    {
        return false;
    }

    return !hogar->estaSatisfecho;
}
```

## 5. Flujo de control

El flujo debe ser explícito y predecible.

- preferir condiciones afirmativas y simples;
- extraer condiciones complejas a variables o funciones con nombre;
- usar `switch` para estados o categorías discretas;
- incluir `default` cuando un valor inválido deba detectarse;
- evitar operadores ternarios anidados;
- evitar modificaciones dentro de condiciones;
- no depender de efectos laterales del cortocircuito para ejecutar acciones;
- usar `goto limpieza` solamente para liberar recursos en una salida de error común;
- no usar recursión si un ciclo expresa la solución de manera más clara.

```c
bool esAccesible = cuotaMensual <= hogar->ingreso * configuracion->rho;
bool esSatisfactoria = evaluarSatisfaccionCandidata(simulacion, hogar, idCelda);

if (esAccesible && esSatisfactoria)
{
    agregarSolicitudMudanza(solicitudes, hogar->id, idCelda);
}
```

## 6. Estructuras y datos

Las estructuras representan conceptos del dominio y no agrupaciones arbitrarias de parámetros.

```c
typedef struct
{
    int id;
    int idCelda;
    ClaseSocial claseSocial;
    double ingreso;
    int mesesBloqueado;
    bool estaSatisfecho;
} Hogar;
```

Reglas:

- incluir `<stdbool.h>`, `<stdint.h>` y `<stddef.h>` cuando correspondan;
- usar tipos enteros de tamaño explícito para datos serializados;
- usar `size_t` para tamaños e índices de memoria;
- usar `double` para pesos, precios, ingresos y métricas;
- inicializar siempre las estructuras antes de usarlas;
- representar identificadores ausentes mediante una constante con nombre;
- evitar números mágicos;
- documentar unidades en la especificación y reflejarlas en nombres cuando exista ambigüedad, como `ingresoMensual`;
- mantener separadas las estructuras de celdas, hogares, configuración y métricas.

## 7. Comentarios

El código debe necesitar pocos comentarios. Los nombres y la separación en funciones deben explicar qué ocurre.

Se permiten comentarios únicamente para indicar:

- una razón que no sea evidente;
- una restricción del modelo;
- una condición necesaria de MPI u OpenMP;
- una decisión de rendimiento comprobada;
- una fórmula cuya fuente o transformación no resulte obvia.

Los comentarios se escriben en minúsculas, sin puntuación final y en una sola frase breve.

```c
// la vivienda liberada queda disponible en la siguiente iteracion
celdaLiberada->disponibleDesdeIteracion = iteracion + 1;
```

```c
// solo el thread principal realiza llamadas mpi
MPI_Allreduce(metricasLocales, metricasGlobales, cantidadMetricas, MPI_DOUBLE, MPI_SUM,
              MPI_COMM_WORLD);
```

No se permiten comentarios que repitan el código:

```c
// aumenta el contador
cantidadVecinos++;
```

Tampoco se dejarán bloques de código comentado, diarios de cambios, explicaciones dirigidas al docente, comentarios generados automáticamente ni marcadores pendientes en la entrega.

Cuando sea posible, los comentarios también usarán caracteres ASCII para evitar problemas de codificación entre máquinas. Esto significa escribir `iteracion` y no `iteración` dentro del código. La documentación Markdown y LaTeX sí conserva tildes y puntuación normal.

## 8. Headers y módulos

Cada módulo tendrá un propósito reconocible y, normalmente, un `.c` y un `.h` con el mismo nombre.

Un header debe contener solamente la interfaz necesaria para otros módulos:

- include guard;
- includes mínimos requeridos por sus declaraciones;
- tipos públicos;
- prototipos públicos;
- constantes públicas indispensables.

No se colocan implementaciones en headers, salvo funciones `static inline` justificadas. No se exponen estructuras internas si los consumidores no necesitan conocer sus campos.

Orden de includes en archivos `.c`:

1. header propio;
2. headers del estándar de C;
3. MPI u OpenMP;
4. headers del proyecto.

Cada grupo se separa con una línea en blanco.

## 9. Errores y recursos

- comprobar resultados de asignación de memoria, apertura de archivos y operaciones críticas;
- devolver códigos de error claros y consistentes;
- escribir errores en `stderr`;
- incluir contexto suficiente en el mensaje sin imprimir datos innecesarios;
- liberar en orden inverso al de adquisición;
- dejar el objeto en un estado válido si una inicialización falla;
- comprobar códigos de retorno MPI cuando la operación pueda manejarse o diagnosticarse;
- usar `MPI_Abort` solo cuando el proceso distribuido no pueda continuar coherentemente;
- no ignorar errores mediante conversiones a `void` salvo justificación explícita.

```c
FILE *archivoEntrada = fopen(ruta, "r");

if (archivoEntrada == NULL)
{
    fprintf(stderr, "no se pudo abrir el archivo de entrada %s\n", ruta);
    return errorArchivoEntrada;
}
```

## 10. Memoria y punteros

- inicializar punteros con `NULL`;
- comprobar `NULL` antes de desreferenciar cuando el contrato lo permita;
- evitar aritmética de punteros si un índice expresa mejor la intención;
- no devolver punteros a variables locales;
- indicar mediante nombres y API qué módulo crea y destruye un objeto;
- proporcionar funciones simétricas como `crearSimulacion()` y `destruirSimulacion()` cuando corresponda;
- evitar asignaciones pequeñas dentro de ciclos críticos;
- no usar variables globales mutables;
- no ocultar asignaciones mediante macros.

## 11. MPI y OpenMP

El código paralelo debe parecerse lo máximo posible al algoritmo secuencial.

- separar cálculo local, comunicación y consolidación;
- solo el thread principal llama a MPI bajo `MPI_THREAD_FUNNELED`;
- no colocar llamadas MPI dentro de regiones OpenMP sin una justificación explícita;
- usar nombres como `cantidadHogaresLocales` y `cantidadHogaresGlobales`;
- usar buffers por thread antes que secciones `critical` en recorridos frecuentes;
- indicar explícitamente `shared`, `private`, `firstprivate` o `reduction` cuando mejore la claridad;
- no depender del orden de planificación de OpenMP;
- agrupar y nombrar tags MPI;
- mantener el orden de colectivas idéntico en todos los procesos;
- comprobar que la versión paralela conserva la semántica de la secuencial antes de optimizar.

Las directivas pueden superar el límite orientativo de 100 caracteres si dividirlas vuelve más difícil leer sus cláusulas.

## 12. Pruebas

Las pruebas respetan el mismo estilo que el código de producción.

- cada prueba verifica un comportamiento concreto;
- los nombres describen escenario y resultado esperado;
- usar arreglos pequeños y valores visibles;
- evitar lógica compleja dentro de la propia prueba;
- ante un fallo, mostrar el valor esperado y el obtenido;
- no compartir estado mutable entre pruebas;
- no depender del orden de ejecución.

```c
static void hogarAisladoEstaSatisfecho(void)
{
    MundoPrueba mundo = crearMundoPruebaVacio(3, 3);
    int idHogar = agregarHogarPrueba(&mundo, 1, 1, claseSocialMedia);

    bool estaSatisfecho = evaluarSatisfaccionHogar(&mundo.simulacion, idHogar);

    verificarVerdadero(estaSatisfecho);
    destruirMundoPrueba(&mundo);
}
```

## 13. Código que se evitará

- funciones genéricas sin relación clara con el dominio;
- macros que actúan como funciones;
- abstracciones creadas para un único uso simple;
- jerarquías o patrones trasladados desde orientación a objetos;
- microoptimizaciones sin mediciones;
- duplicación entre versión secuencial e híbrida cuando la regla del modelo puede compartirse;
- números mágicos y cadenas repetidas;
- comentarios extensos para compensar código difícil;
- funciones con nombres vagos como `procesarDatos`, `manejarCosas` o `hacerTrabajo`;
- cambios de formato manuales que contradigan la guía.

## 14. Validación del estilo

El repositorio incluirá una configuración de `clang-format` compatible con estas reglas. Antes de integrar cambios se ejecutarán:

```bash
clang-format --dry-run --Werror archivos.c archivos.h
cppcheck --enable=warning,performance,portability src include tests
```

El formateador ayuda a mantener consistencia, pero no reemplaza la revisión de nombres, responsabilidades, simplicidad ni legibilidad.

## 15. Criterio para resolver dudas

Cuando existan dos soluciones correctas, se elegirá la que:

1. sea más fácil de explicar oralmente;
2. muestre de forma más directa la regla del modelo;
3. tenga menos estados ocultos y efectos secundarios;
4. sea verificable con una prueba pequeña;
5. mantenga el mismo comportamiento en secuencial y paralelo.

La brevedad no es un objetivo por sí sola. Unas pocas líneas adicionales son preferibles si hacen evidente el comportamiento.
