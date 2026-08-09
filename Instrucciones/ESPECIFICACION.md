# Especificación de implementación: modelo de Schelling para Montevideo

Estado: versión 1 aprobada para iniciar la implementación  
Fecha de cierre de decisiones: 9 de agosto de 2026  
Base documental: `Informe_Inicial_Arce_Ferrero.pdf` y material del curso HPC 2026  
Lenguaje y tecnologías: C, MPI y OpenMP

La implementación debe respetar además las convenciones de `Instrucciones/ESTILO_C.md`. Ante una contradicción, esta especificación define el comportamiento y la guía de estilo define únicamente su presentación en el código.

## 1. Objetivo

Implementar una simulación reproducible de segregación residencial basada en Schelling y aplicada a una representación simplificada de Montevideo. El sistema debe ofrecer:

1. una versión secuencial de referencia;
2. una versión paralela híbrida, con descomposición espacial mediante MPI y paralelismo local mediante OpenMP;
3. resultados funcionalmente equivalentes entre ambas versiones para una misma entrada y semilla;
4. instrumentación para evaluar tiempo, speedup, eficiencia, escalabilidad fuerte y balance de carga.

El modelo es una herramienta experimental. No pretende predecir la evolución residencial real de Montevideo.

## 2. Alcance

### 2.1 Alcance obligatorio

- Grilla bidimensional con celdas residenciales y no residenciales.
- Viviendas residenciales ocupadas o vacías, una vivienda como máximo por celda.
- Hogares pertenecientes a siete subestratos (`A+`, `A-`, `M+`, `M`, `M-`, `B+`, `B-`) y tres clases (`alta`, `media`, `baja`).
- Ingreso por hogar derivado de su subestrato.
- Vecindario configurable, ponderado por distancia.
- Satisfacción según una matriz configurable de preferencias y tolerancias.
- Precios de vivienda y restricción de capacidad de pago.
- Permanencia mínima luego de una mudanza.
- Búsqueda de vivienda, solicitudes, resolución de conflictos y aplicación simultánea de mudanzas.
- Métricas del modelo y de desempeño.
- Entrada sintética reproducible; la importación de datos geográficos reales debe poder incorporarse sin cambiar el motor.
- Checkpoints básicos y reinicio desde un estado guardado.

### 2.2 Fuera del alcance inicial

- Predicción o validación causal sobre la ciudad real.
- Interfaz gráfica interactiva.
- CUDA/GPU, Hadoop o servicios cloud.
- Versiones independientes solo-MPI y solo-OpenMP como entregables principales.
- Particiones espaciales irregulares en la primera versión funcional.
- Barrido exhaustivo de parámetros.

## 3. Modelo de dominio

### 3.1 Grilla y celdas

La grilla tiene `width * height` celdas, almacenadas de forma contigua por filas. Cada celda contiene, como mínimo:

- tipo: residencial o no residencial;
- estado: ocupada o vacía, si es residencial;
- identificador del hogar ocupante, o un valor centinela;
- zona administrativa;
- precio actual de la vivienda.

Los identificadores de celdas y hogares deben ser globales, enteros y estables durante toda la ejecución. Las coordenadas se derivan del identificador de celda cuando sea conveniente.

### 3.2 Hogar

Cada hogar contiene, como mínimo:

- identificador global;
- celda actual;
- subestrato y clase socioeconómica;
- ingreso mensual;
- meses restantes de bloqueo por permanencia mínima;
- estado de satisfacción de la iteración actual.

Distribución base de subestratos para Montevideo, tomada de la Tabla D3 del INSE 2023:

| Clase | Subestrato | Porcentaje |
|---|---:|---:|
| Alta | A+ | 8,6 % |
| Alta | A- | 17,1 % |
| Media | M+ | 21,2 % |
| Media | M | 21,8 % |
| Media | M- | 16,0 % |
| Baja | B+ | 10,4 % |
| Baja | B- | 5,0 % |

Multiplicadores e ingresos base, expresados en miles de pesos corrientes de 2022:

| Subestrato | Multiplicador | Ingreso |
|---|---:|---:|
| A+ | 3,7 | 122,5 |
| A- | 2,1 | 71,0 |
| M+ | 1,4 | 46,7 |
| M | 1,0 | 33,2 |
| M- | 0,7 | 23,7 |
| B+ | 0,5 | 16,1 |
| B- | 0,3 | 9,9 |

La escala objetivo del caso completo es aproximadamente 580.000 viviendas, 520.000 ocupadas y entre 55.000 y 58.000 vacías. Deben existir instancias pequeñas para pruebas.

El informe inicial utilizaba `B- 8,7 %`, `B+ 15,9 %`, `M- 15,1 %`, `M 19,6 %`, `M+ 20,1 %`, `A- 14,3 %` y `A+ 6,3 %`. Esa distribución se conserva únicamente como escenario alternativo para reproducir el planteo inicial; no es la configuración base.

### 3.3 Vecindario

Para una celda residencial `c`, el vecindario de radio `r` es:

`N_r(c) = {c' residencial : distancia(c,c') <= r}`

La pertenencia al vecindario y la distancia usada para ponderar se configuran por separado. La configuración base usa un vecindario Chebyshev de radio `r = 2`, es decir, un cuadrado de `5 x 5` sin la propia celda y con hasta 24 vecinos. La propia celda evaluada nunca se cuenta como vecina.

El peso base de un vecino es el kernel gaussiano, calculado con distancia euclidiana aunque la pertenencia al vecindario se determine mediante Chebyshev:

`w(c,c') = exp(-(distancia(c,c')^2) / (2*sigma^2))`

La configuración base usa `sigma = 1,0`. Los pesos para cada desplazamiento relativo deben precalcularse al iniciar la simulación. Tanto `r` como `sigma` serán parámetros del escenario. Se incluirá como comparación un escenario clásico con `r = 1` y pesos uniformes.

La grilla tiene bordes cerrados, no periódicos. Un vecindario que excede un límite se recorta; las posiciones exteriores no existen ni participan en denominadores. La búsqueda usa distancia directa dentro del rectángulo y no puede atravesar un borde para reaparecer en el opuesto. Las métricas por zona permitirán observar posibles efectos de borde.

La proporción ponderada de la clase `j` se obtiene dividiendo su suma de pesos por la suma de pesos de todos los hogares vecinos. Las celdas vacías y no residenciales no contribuyen al denominador.

Si no existe ningún hogar vecino, las proporciones no se calculan y el hogar se considera satisfecho por aislamiento. Esta excepción se aplica tanto a su vivienda actual como a la evaluación hipotética de una vivienda candidata. La métrica de salida debe distinguir estos casos de los hogares que satisfacen efectivamente la matriz de tolerancias.

### 3.4 Satisfacción

Un hogar está satisfecho únicamente si cumple simultáneamente todas las restricciones de su fila:

| Hogar evaluado | Clase alta | Clase media | Clase baja |
|---|---:|---:|---:|
| Alta | >= 70 % | <= 30 % | <= 5 % |
| Media | <= 70 % | >= 50 % | <= 40 % |
| Baja | <= 80 % | <= 90 % | >= 2 % |

La matriz debe cargarse desde configuración y no quedar codificada en la lógica del algoritmo.

### 3.5 Precio y accesibilidad

El precio de una vivienda vacía se actualiza mediante:

`log(precio_v(t)) = alpha0 + beta1 * D_v(t) + beta2 * A_v(t) + epsilon_v(t)`

La demanda local es la proporción ponderada de viviendas residenciales ocupadas:

`D_v(t) = suma(w(v,c) * ocupada(c)) / suma(w(v,c) * residencial(c))`

Las celdas no residenciales no participan. Si no existen celdas residenciales vecinas, `D_v(t) = 0`.

El poder adquisitivo local parte del ingreso medio ponderado de los hogares vecinos y se normaliza con los ingresos de referencia mínimo (`B-`) y máximo (`A+`):

`ingreso_medio_v = suma(w(v,c) * ingreso(c)) / suma(w(v,c) * ocupada(c))`

`A_v(t) = clamp((ingreso_medio_v - ingreso_B-) / (ingreso_A+ - ingreso_B-), 0, 1)`

Si no existen hogares vecinos, `A_v(t) = 0`. Ambas variables usan el vecindario y los pesos definidos en la sección 3.3 y quedan en `[0,1]`.

El término `epsilon_v(t)` se genera como una normal de media `0` y desviación estándar `0,05`, truncada a `[-0,15; 0,15]`. Se aplica sobre el logaritmo del precio y se deriva determinísticamente de `semilla`, `iteración`, `id_vivienda` y una etiqueta exclusiva para ruido de precio. Se genera un valor nuevo cada vez que corresponde actualizar esa vivienda y no depende del proceso, thread ni orden de ejecución. Debe existir un escenario de control con `epsilon_v(t) = 0`.

El proyecto implementará un generador pseudoaleatorio propio, pequeño y sin estado compartido. Una función de mezcla de 64 bits recibirá una clave formada por `semilla`, `iteración`, identificador de entidad, propósito y número de muestra, y producirá bits reproducibles. Estos bits se convertirán primero en uniformes en el intervalo abierto `(0,1)` y, para el ruido normal, se aplicará la transformación de Box--Muller. No se utilizará `rand()` ni secciones críticas para aleatoriedad. El generador tendrá vectores de prueba fijos y pruebas de equivalencia entre configuraciones MPI/OpenMP; no se considera apto para criptografía.

La vivienda es accesible para el hogar `h` cuando:

`cuota(precio_v) <= rho * ingreso_h`

El precio y los ingresos se expresan en miles de pesos constantes de 2022. La cuota mensual se aproxima como una anualidad de cuota fija:

`capital = fraccion_financiada * precio_v`

`cuota(precio_v) = capital * (i * (1 + i)^n) / ((1 + i)^n - 1)`

donde la configuración base usa una entrega inicial de `20 %` (`fraccion_financiada = 0,80`), tasa nominal anual fija de `6 %` (`i = 0,06 / 12`) y plazo de `240` meses. Si `i = 0`, se usa `cuota = capital / n`. Entrega, tasa y plazo son configurables. El cálculo no incluye seguros, gastos, inflación ni unidades indexadas y se documenta como aproximación de capacidad económica, no como producto hipotecario real.

La configuración económica base usa `beta1 = 0,40`, `beta2 = 0,60` y `rho = 0,30`. Por lo tanto, el poder adquisitivo local tiene mayor influencia que la ocupación y ningún hogar puede destinar más del 30 % de su ingreso a la cuota.

`alpha0` se calibra, no se introduce como un valor arbitrario. Se toma como referencia un hogar `M`, una demanda local `D_ref = 0,90`, el poder adquisitivo normalizado correspondiente a su ingreso (`A_M = 0,206927...`) y una cuota objetivo igual al 25 % de su ingreso. Con los parámetros financieros base, el factor cuota/precio es `0,005731448...`, el precio de referencia es `1448,150506` miles de pesos de 2022 y:

`alpha0 = log(precio_ref) - beta1 * D_ref - beta2 * A_M = 6,793886203`

Este valor produce una cuota objetivo de `8,3` miles de pesos para el hogar `M`, inferior a su máximo de `rho * 33,2 = 9,96`. Si cambian ingresos, financiación, tasa, plazo, coeficientes o entorno de referencia, el preprocesador debe recalcular `alpha0` y registrar el resultado efectivo.

El ruido debe generarse de manera reproducible. Los precios se calculan a partir del estado consolidado al comienzo de la iteración y se aplican antes de buscar destinos.

### 3.6 Mudanzas

Un hogar puede solicitar una mudanza si está insatisfecho y su bloqueo es cero. El proceso lógico de una iteración es:

1. evaluar satisfacción sobre una instantánea inmutable;
2. actualizar precios de viviendas vacías;
3. identificar hogares habilitados;
4. filtrar destinos vacíos y económicamente accesibles;
5. evaluar satisfacción hipotética en cada destino;
6. elegir el destino satisfactorio de menor distancia;
7. generar como máximo una solicitud por hogar;
8. resolver conflictos, como máximo un ganador por vivienda;
9. aplicar todas las mudanzas confirmadas;
10. actualizar bloqueos y métricas.

En igualdad de distancia se elige el menor identificador global de vivienda. Ante varias solicitudes para una vivienda, gana el hogar con mayor capacidad de pago; si empatan, gana la menor clave pseudoaleatoria determinista y luego el menor identificador global.

Una vivienda liberada en la iteración `t` no puede utilizarse hasta `t+1`. Un hogar trasladado queda bloqueado durante la permanencia mínima configurada.

La permanencia mínima base es de 12 meses. Todos los hogares comienzan con bloqueo cero. Si un hogar se muda en la iteración `t`, no puede solicitar otra mudanza durante `t+1, ..., t+12` y vuelve a ser elegible en `t+13`. Su satisfacción continúa calculándose durante el bloqueo y las métricas distinguen hogares insatisfechos inmovilizados por esta regla. El valor es configurable y se incluirá un escenario de control con permanencia cero.

Si no existe un destino satisfactorio, la política base es permanecer en la vivienda actual. La alternativa de elegir el destino con mayor mejora se podrá implementar después como política configurable.

## 4. Semántica temporal y determinismo

La simulación es síncrona: todas las decisiones de una iteración leen el mismo estado inicial y las modificaciones se consolidan al final. Esta regla evita que el orden de recorrido altere los resultados.

No se usará `rand()` ni un generador con estado global compartido. Toda decisión aleatoria se derivará de una función determinista de, al menos, `semilla`, `iteración`, `id_hogar` y propósito de la decisión. Esto debe hacer el resultado independiente del número de procesos MPI, threads y planificación de OpenMP.

Para una misma entrada, configuración y semilla, la versión secuencial y la híbrida deben producir el mismo hash del estado consolidado en cada iteración. Los cálculos flotantes colectivos deben diseñarse cuidadosamente; para validación se admitirán tolerancias documentadas solamente en métricas agregadas, no en ocupación ni decisiones.

## 5. Arquitectura del código

Estructura propuesta:

```text
include/
  schelling/*.h
src/
  common/       modelo, configuración, E/S, RNG, métricas, checkpoint
  sequential/   controlador secuencial
  parallel/     descomposición MPI, halos, migraciones y OpenMP
app/
  schelling_seq.c
  schelling_hybrid.c
tests/
  unit/
  integration/
config/
  base.conf
scripts/
  benchmark.sh
  validate.sh
output/         ignorado, salvo .gitkeep
results/        ignorado, salvo .gitkeep
```

El motor común debe concentrar reglas del modelo para evitar dos implementaciones divergentes. La versión paralela cambia la distribución y coordinación de datos, no la semántica.

### 5.1 Representación y rendimiento

- Preferir arreglos contiguos y estructuras simples en los recorridos críticos.
- Separar datos de grilla de datos de hogares.
- Mantener listas compactas de viviendas vacías y hogares activos.
- Precalcular desplazamientos y pesos del vecindario.
- Implementar primero un recálculo completo correcto.
- Incorporar después el recálculo incremental de vecindarios afectados, validándolo contra el completo.
- Evitar búsqueda global ingenua de todas las viviendas en la escala final; usar índices espaciales por bloques o zonas y expansión por distancia.

## 6. Interfaz de ejecución

Binarios previstos:

```bash
./build/schelling_seq --config config/base.conf --seed 42
mpirun -np 4 ./build/schelling_hybrid --config config/base.conf --seed 42
```

Opciones mínimas:

- `--config PATH`: archivo de escenario;
- `--seed N`: semilla reproducible;
- `--iterations N`: sobrescribe la cantidad de meses;
- `--input PATH`: estado o conjunto de datos inicial;
- `--output PATH`: directorio de salida;
- `--checkpoint-every N`: periodicidad, cero para desactivar;
- `--restart PATH`: reinicio desde checkpoint;
- `--validate`: genera hashes y controles adicionales;
- `--help` y `--version`.

La configuración debe incluir dimensiones, ocupación, zonas, distribuciones, radio, distancia, `sigma`, tolerancias, `alpha0`, `beta1`, `beta2`, distribución de ruido, `rho`, permanencia mínima, iteraciones y política de búsqueda.

Los errores de entrada deben terminar con código distinto de cero y un mensaje claro. Todos los parámetros efectivos deben copiarse a la salida del experimento.

## 7. Versión secuencial

La versión secuencial es la referencia funcional y la base `T1` para speedup. Debe implementarse y validarse antes de introducir MPI u OpenMP.

Orden de desarrollo:

1. carga y validación de configuración;
2. generación/carga del estado inicial;
3. cálculo de vecindario y satisfacción;
4. actualización de precio y accesibilidad;
5. solicitudes, conflictos y mudanzas síncronas;
6. métricas y salida;
7. checkpoint/reinicio;
8. optimización e índices de búsqueda.

## 8. Versión híbrida MPI + OpenMP

### 8.1 MPI

El modelo será SPMD. Cada proceso posee un subdominio rectangular y sus hogares, viviendas y vacantes. La primera partición será por franjas o bloques rectangulares; posteriormente se comparará área igual con cortes balanceados por cantidad de viviendas u hogares.

Cada proceso mantiene un halo de ancho `r`. El intercambio debe realizarse con procesos vecinos después de consolidar cada iteración y antes del siguiente cálculo de satisfacción. Se priorizan comunicaciones no bloqueantes y espera conjunta para permitir solapamiento cuando sea posible.

La búsqueda remota intercambia únicamente datos necesarios de vacantes: identificador, dueño MPI, coordenadas, precio y disponibilidad. Las solicitudes se enrutan al proceso dueño. Este resuelve conflictos determinísticamente y comunica aceptaciones. Luego los hogares que cruzan subdominios migran al nuevo dueño.

Las métricas globales se obtienen con colectivas MPI. El tiempo de una fase paralela es el máximo entre procesos, no el promedio.

### 8.2 OpenMP

Dentro de cada proceso se paralelizan bucles independientes de:

- satisfacción y proporciones ponderadas;
- actualización local de precios;
- evaluación de destinos;
- construcción de solicitudes en buffers por thread;
- métricas locales mediante reducciones.

Los threads leen una instantánea inmutable y escriben en posiciones exclusivas o buffers privados. No deben modificar directamente la grilla durante la fase paralela. Se evitarán secciones `critical` en recorridos calientes; se preferirán reducciones, buffers por thread y combinación determinista posterior.

Se compilará con soporte OpenMP (`-fopenmp`) y MPI mediante el wrapper `mpicc`. El proceso MPI debe solicitar/verificar un nivel de soporte de threads compatible con el uso realizado; inicialmente, solo el thread principal efectuará llamadas MPI (`MPI_THREAD_FUNNELED`).

## 9. Compilación y calidad

Se utilizará C11 como base portable y CMake como sistema de construcción. Perfiles mínimos:

- desarrollo: advertencias estrictas, símbolos de depuración y sanitizadores cuando estén disponibles;
- release: optimización (`-O3`) sin desactivar controles que cambien la semántica;
- secuencial: no requiere runtime MPI;
- híbrido: `mpicc` y OpenMP.

El proyecto debe compilar sin advertencias con GCC en el ambiente objetivo. No se permiten variables globales mutables para el estado de simulación ni fugas de memoria detectables en las pruebas pequeñas.

## 10. Entradas y salidas

### 10.1 Entrada

La primera implementación usará un formato simple y documentado (CSV y archivo de configuración). Debe soportar:

- generación sintética reproducible;
- carga de celdas/zonas;
- carga opcional de hogares y viviendas;
- validación de identificadores, rangos, duplicados y consistencia de ocupación.

La adaptación de fuentes INE/INSE se hará en herramientas de preprocesamiento separadas del simulador.

### 10.2 Salida

Cada ejecución crea:

- `run.json` o equivalente: configuración efectiva, semilla, versión Git, compilador, host, procesos y threads;
- `metrics.csv`: una fila por iteración;
- `timings.csv`: tiempos total y por fase;
- estado final o su hash;
- checkpoints, si están habilitados;
- errores por `stderr`.

Métricas funcionales mínimas:

- hogares satisfechos total y por clase;
- mudanzas solicitadas, aceptadas y rechazadas;
- hogares bloqueados y sin destino asequible;
- población por clase y zona;
- precio promedio por zona;
- viviendas ocupadas y vacías.

## 11. Validación y pruebas

### 11.1 Pruebas unitarias

- conversión celda/coordenadas y distancias;
- generación y normalización de pesos;
- exclusión del propio hogar y celdas vacías;
- cada condición de la matriz de tolerancias, incluidos límites exactos;
- accesibilidad económica;
- elección por distancia y desempates;
- conflicto entre hogares por una vacante;
- bloqueo y reactivación por permanencia mínima;
- RNG determinista;
- serialización y restauración de checkpoint.

### 11.2 Pruebas de integración

- grillas manuales pequeñas con resultado esperado;
- conservación: hogares + vacantes residenciales constante;
- ninguna vivienda contiene más de un hogar;
- cada hogar ocupa exactamente una vivienda;
- celdas no residenciales nunca se ocupan;
- una vivienda liberada no se reutiliza en la misma iteración;
- reinicio produce el mismo resultado que una ejecución ininterrumpida;
- secuencial e híbrida producen el mismo hash por iteración con 1, 2 y más recursos;
- resultados idénticos al variar `OMP_NUM_THREADS` y la planificación.

### 11.3 Criterio de aceptación funcional

El hito funcional se acepta cuando todas las pruebas anteriores pasan, no hay errores de memoria en instancias pequeñas y ambas versiones coinciden durante al menos un escenario pequeño, uno mediano y el escenario base factible en el entorno disponible.

## 12. Evaluación experimental

Las mediciones deben realizarse en condiciones comparables, registrando hardware, software, opciones de compilación y afinidad. Se recomienda una ejecución de calentamiento y al menos cinco repeticiones medidas por configuración; reportar mediana y dispersión.

Configuraciones iniciales propuestas por el informe:

| Procesos MPI | Threads OpenMP | Recursos totales |
|---:|---:|---:|
| 1 | 1 | 1 |
| 1 | 4 | 4 |
| 2 | 4 | 8 |
| 4 | 4 | 16 |
| 8 | 4 | 32 |

Se calculará:

- tiempo total y por iteración;
- tiempo por fase: satisfacción, precios, búsqueda, conflictos, comunicación, aplicación y métricas;
- speedup algorítmico `S_p = T_seq / T_p`;
- eficiencia `E_p = S_p / p`, donde `p` es la cantidad total de recursos de cómputo asignados;
- indicador de desbalance `L = max(T_i) / promedio(T_i)`;
- volumen de comunicación y trabajo por proceso.

La evaluación principal será de escalabilidad fuerte: mismo problema y semilla, aumentando recursos. Además se comparará partición geométrica uniforme contra partición balanceada por cantidad de viviendas u hogares.

El tiempo de inicialización/E/S se reportará separado y también se mantendrá un tiempo total de extremo a extremo. Las métricas funcionales no se incluirán dentro de la región cronometrada si en producción pueden desactivarse; cualquier exclusión debe ser idéntica entre versiones.

## 13. Checkpoints

Un checkpoint contiene versión de formato, iteración, configuración efectiva, semilla, grilla, ocupación, hogares, precios, bloqueos y estado necesario del RNG. Se escribe primero a un archivo temporal y se publica al completarse para evitar estados parciales.

En la versión híbrida cada proceso puede escribir una parte y el proceso raíz un manifiesto. Solo se considera válido un checkpoint si todas las partes declaradas existen y pasan sus controles de integridad.

La ejecución normal crea un checkpoint cada 50 iteraciones y conserva los dos checkpoints completos más recientes. El estado final se guarda por separado y no cuenta para esa retención. La frecuencia y cantidad retenida son configurables. La publicación es transaccional: cada proceso escribe primero una parte temporal, se verifican todas las partes y sus checksums, y el proceso raíz publica al final el manifiesto que vuelve válido al checkpoint. Al reiniciar se validan versión de formato, configuración compatible, manifiesto, cantidad de partes y checksums.

Los benchmarks oficiales usan frecuencia cero, es decir, checkpoints desactivados, para no incluir E/S de tolerancia a fallos en los tiempos del algoritmo. El costo de checkpoint podrá medirse en un experimento separado.

## 14. Plan incremental

### Hito 0: esqueleto

- CMake, estructura, configuración, logging y pruebas automáticas.
- Binario secuencial que carga un escenario mínimo.

### Hito 1: modelo secuencial correcto

- Grilla, hogares, vecindario, satisfacción y simulación síncrona.
- Precios, capacidad económica, mudanzas y métricas.
- Pruebas de invariantes y casos manuales.

### Hito 2: reproducibilidad y persistencia

- RNG sin estado compartido, hash por iteración y salida experimental.
- Checkpoint y reinicio.

### Hito 3: optimización secuencial

- Pesos precalculados, listas compactas e índice espacial de vacantes.
- Perfilado y línea base `T1`.

### Hito 4: MPI

- Partición rectangular, halos, solicitudes remotas y migración.
- Equivalencia con la versión secuencial.

### Hito 5: OpenMP e híbrido

- Bucles locales paralelos, buffers por thread y afinidad.
- Pruebas de carreras y determinismo.

### Hito 6: experimentos

- Automatización de repeticiones y configuraciones.
- Speedup, eficiencia, desbalance y análisis funcional.

### Hito 7: extensiones condicionadas al tiempo

- Actualización incremental de vecindarios.
- Particiones balanceadas y/o irregulares.
- Política de mudanza por mayor mejora.
- Múltiples escenarios ejecutados en paralelo.

## 15. Registro de decisiones previas al Hito 1

El informe inicial dejó abiertas las siguientes definiciones. El equipo las resolvió en conversación y esta lista constituye el registro normativo que debe seguir la implementación:

1. **Resuelto:** dimensiones, geometría y significado espacial de la grilla;
2. **Resuelto:** formato, jerarquía y versiones de las fuentes de datos;
3. **Resuelto:** vecindario Chebyshev con `r = 2`, ponderación gaussiana por distancia euclidiana y `sigma = 1,0`;
4. **Resuelto:** un hogar sin hogares vecinos se considera satisfecho por aislamiento, también al evaluar un destino;
5. **Resuelto:** `D_v` es ocupación residencial ponderada y `A_v` es ingreso medio ponderado normalizado entre `B-` y `A+`, ambos en `[0,1]`;
6. **Resuelto:** normal `N(0; 0,05)` truncada a `[-0,15; 0,15]`, determinista por vivienda e iteración, con escenario sin ruido;
7. **Resuelto:** anualidad sobre 80 % financiado, tasa nominal anual de 6 % y plazo de 240 meses, con parámetros configurables;
8. **Resuelto:** `beta1 = 0,40`, `beta2 = 0,60`, `rho = 0,30` y `alpha0 = 6,793886203`, calibrado para una cuota del 25 % del ingreso `M` en el entorno de referencia;
9. **Resuelto:** 12 meses posteriores a una mudanza, con valor configurable y escenario de control igual a cero;
10. **Resuelto:** cada 50 iteraciones y retención de los dos últimos en ejecuciones normales; desactivados en benchmarks;
11. **Resuelto:** bordes cerrados, sin conexión periódica; vecindarios recortados en los límites;
12. **Resuelto:** C11/GCC, MPI portable, OpenMP, CMake principal y Makefile independiente; pruebas locales con MPICH y pruebas distribuidas en FING cargando `mpi/mpich-x86_64`.

Las pruebas usarán valores sintéticos declarados en sus archivos de escenario cuando no requieran el conjunto de Montevideo. Ninguna decisión debe quedar implícita en el código: los valores efectivos se cargan desde configuración o se registran expresamente como constantes del formato/modelo.

### 15.1 Decisión para los puntos 1 y 2

La representación espacial y los datos quedan definidos así:

1. usar una grilla rectangular abstracta, como en el modelo clásico de Schelling; sus coordenadas representan proximidad en la simulación y no coordenadas geográficas reales;
2. usar `1024 x 640` celdas para el escenario completo inicial, con dimensiones sobrescribibles desde configuración; las 655.360 posiciones permiten representar aproximadamente 580.000 viviendas y reservar el resto como celdas no residenciales;
3. representar los ocho municipios como zonas contiguas de la grilla, asignando sus áreas discretas proporcionalmente al stock oficial de viviendas, sin intentar reproducir sus polígonos ni límites reales;
4. distribuir en cada zona las viviendas ocupadas y vacías conservando exactamente los totales oficiales de la versión de datos seleccionada;
5. asignar hogares por subestrato usando como base la distribución del INSE específica para Montevideo; una distribución por barrio solo se incorporará como escenario posterior si la calidad de los datos lo permite;
6. producir archivos CSV normalizados para C; el simulador no dependerá de bibliotecas GIS;
7. conservar generadores sintéticos a escalas pequeña, mediana y grande para pruebas y benchmarks independientes de los datos reales.

Esta es una simplificación deliberada: el sistema estudia dinámicas de proximidad y movilidad entre zonas parametrizadas con datos de Montevideo, pero no reproduce distancias, formas, calles, costas ni posiciones reales. Por lo tanto, los resultados espaciales no deben presentarse como mapas predictivos de la ciudad.

Fuentes candidatas y control de versión:

- **INE, Censo 2023 ponderado, versión mayo 2026:** microdatos anonimizados y tabulados de hogares/población. Es la fuente preferida para totales actuales.
- **INE, caracterización de viviendas desocupadas (enero 2025):** reporta 55.630 viviendas desocupadas en Montevideo y 9,3 % del stock en esa publicación. Debe registrarse la versión porque otra nota del mismo período informa 9,6 %.
- **CEISMU, INSE 2023 sobre ECH 2022:** distribución por subestrato e ingresos per cápita con valor locativo. Para Montevideo se usará la fila específica de la Tabla D3 y, si se inicializa por barrio, la Tabla D5.
- **Intendencia de Montevideo, información por municipio basada en ECH 2023:** aporta indicadores socioeconómicos municipales, pero sus cantidades de viviendas por municipio corresponden a los censos 2004 y 2011; no deben presentarse como cantidades del Censo 2023.
- **Cartografía del INE e Intendencia de Montevideo:** fuentes de contraste y extensiones futuras; no se requieren para construir la grilla rectangular base.

Al 9 de agosto de 2026, el INE indica que la cartografía 2023 fue actualizada en mayo de 2026, pero también advierte una revisión técnica por diferencias puntuales entre conteos territoriales y fuentes espaciales. Esta advertencia no bloquea el escenario rectangular, porque este usa totales tabulares; sí debe considerarse si en el futuro se incorpora geometría real.

El criterio no es que una celda tenga una superficie geográfica fija: una celda residencial representa una vivienda discreta del simulador. Las dimensiones `1024 x 640` también favorecen particiones regulares con distintas cantidades de procesos. Se incluirán al menos dos tamaños menores y, si los recursos lo permiten, uno mayor para estudiar sensibilidad y escalabilidad.

### 15.2 Decisión para el punto 12

La implementación usa C11 portable y GCC como compilador de referencia. No se usarán extensiones específicas de Open MPI o MPICH: el código MPI deberá compilar con ambas implementaciones y OpenMP se habilitará mediante `-fopenmp`.

CMake será el sistema de construcción principal y existirá además un Makefile independiente y sencillo para las máquinas de FING, por si CMake no está disponible. Ambos deben ofrecer compilación secuencial e híbrida, pruebas y perfiles equivalentes de desarrollo y release. La integración automatizada debe detectar divergencias entre los dos sistemas.

El entorno local de referencia es Ubuntu 24.04 x86-64 con GCC, MPICH, CMake y herramientas de depuración/análisis. En FING, el material del curso indica cargar `module load mpi/mpich-x86_64`, compilar con `mpicc` y ejecutar con `mpirun`, usando `-hosts` o `-hostfile` para múltiples máquinas. Los nombres de hosts, usuarios, claves y archivos de acceso no se codifican ni versionan.

Antes de los experimentos se registrarán versiones de compilador y MPI, CPU, memoria, topología, sistema operativo, afinidad y comandos efectivos. La primera validación paralela se realizará localmente con varios procesos en una única computadora; posteriormente se repetirá en las máquinas de FING.

## 16. Definición de terminado

El proyecto se considera completo cuando:

- existen binarios secuencial e híbrido reproducibles;
- ambos implementan la misma semántica y superan las pruebas de equivalencia;
- las entradas, parámetros y salidas están documentados;
- los checkpoints permiten continuar sin alterar el resultado;
- las mediciones se ejecutan mediante scripts repetibles;
- se reportan tiempos, speedup, eficiencia, escalabilidad fuerte y balance;
- se documentan limitaciones del modelo y decisiones finalmente adoptadas;
- un clon limpio puede compilar, probar y ejecutar un ejemplo siguiendo el README.

## 17. Informe vivo y trazabilidad

El informe final se desarrollará en paralelo con el software, usando las fuentes LaTeX versionadas en `informe/`. Tendrá formato IEEE a dos columnas y conservará una estructura similar al informe inicial, ampliada con el diseño finalmente implementado, la validación y la evidencia experimental.

Cada hito de implementación incluye una actualización documental:

| Hito técnico | Contenido que debe actualizarse en el informe |
|---|---|
| Decisiones del modelo | Definición formal, parámetros, supuestos y fuentes de datos |
| Secuencial | Algoritmo, estructuras, pseudocódigo, complejidad y validación |
| Reproducibilidad | Semillas, determinismo, checkpoints y metodología de comparación |
| MPI | Partición, halos, comunicaciones, conflictos y migración de hogares |
| OpenMP | Regiones paralelas, reparto de trabajo, sincronización y carreras evitadas |
| Pruebas | Casos, invariantes, equivalencia y limitaciones conocidas |
| Experimentos locales | Plataforma, configuraciones, tiempos preliminares y análisis |
| Experimentos FING | Infraestructura, compilación, ejecución, repeticiones y resultados finales |

Las afirmaciones importantes deben vincularse con alguno de estos elementos: referencia bibliográfica, decisión explícita del modelo, prueba automatizada o resultado experimental reproducible. Las tablas y figuras generadas por scripts deben conservar el comando y los datos de origen; no se editarán manualmente para cambiar resultados.

La estructura prevista del informe es:

1. resumen y palabras clave;
2. introducción y motivación;
3. definición del problema y modelo;
4. fuentes de datos, preparación e hipótesis;
5. diseño e implementación secuencial;
6. estrategia e implementación híbrida MPI/OpenMP;
7. metodología experimental y plataformas;
8. validación funcional;
9. resultados de desempeño y del modelo;
10. discusión, amenazas a la validez y limitaciones;
11. conclusiones y trabajo futuro;
12. referencias y, si corresponde, anexos de reproducibilidad.

Los apartados se completarán cuando exista evidencia suficiente. Los valores aún no medidos se marcarán con `\textbf{PENDIENTE}` y no se inventarán resultados. Antes de la entrega se debe comprobar que no queden marcadores `PENDIENTE`, referencias rotas ni figuras sin fuente.

El punteo adicional de requisitos del informe provisto por el equipo deberá incorporarse a esta estructura y mantenerse como lista de control hasta la entrega.
