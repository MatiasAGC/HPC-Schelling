# HPC-Schelling

Simulación secuencial y paralela híbrida (MPI + OpenMP) de un modelo de segregación residencial basado en Schelling y parametrizado con datos agregados de Montevideo.

## Estado

La especificación funcional está cerrada y la implementación secuencial funcional del Hito 1 está disponible. Incluye generación sintética reproducible, vecindarios ponderados, satisfacción, precios, accesibilidad, solicitudes, conflictos y mudanzas síncronas. La búsqueda de destinos todavía es la referencia exhaustiva y se optimizará en el Hito 3.

Los documentos principales son:

- `Instrucciones/ESPECIFICACION.md`: requisitos, decisiones del modelo, arquitectura, pruebas y plan incremental.
- `Instrucciones/ESTILO_C.md`: convenciones de escritura y organización del código C.
- `informe/main.tex`: informe vivo en formato IEEE.
- `informe/REQUISITOS.md`: correspondencia entre el punteo de entrega y las secciones del informe.

Los PDF del curso, el informe inicial y otros materiales de referencia dentro de `Instrucciones/` se mantienen localmente y están ignorados por Git.

## Compilar el informe

```bash
make -C informe
```

El resultado se genera en `informe/informe.pdf` y no se versiona. Para listar apartados todavía incompletos:

```bash
make -C informe pendientes
```

## Compilación

La construcción principal utiliza CMake:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

También hay un Makefile independiente para los equipos donde CMake no esté disponible:

```bash
make debug
make test
```

La prueba híbrida requiere una instalación MPI funcional. Los controles que no ejecutan MPI pueden correrse con:

```bash
ctest --test-dir build --output-on-failure -E hibrido
```

## Ejecución actual

```bash
./build/schelling_seq --config config/base.conf --validate
OMP_NUM_THREADS=2 mpirun -np 2 ./build/schelling_hybrid --config config/base.conf --validate
```

El ejecutable secuencial genera el estado inicial y ejecuta la cantidad configurada de iteraciones. El ejecutable híbrido conserva por ahora el arranque validado del Hito 0; la distribución MPI y OpenMP se incorpora en los hitos 4 y 5.

Para consultar todas las opciones:

```bash
./build/schelling_seq --help
```

## Calidad

```bash
make format-check
make analyze
cmake -S . -B build-sanitize -DCMAKE_BUILD_TYPE=Debug -DHABILITAR_SANITIZADORES=ON
cmake --build build-sanitize --parallel
ASAN_OPTIONS=detect_leaks=0 ctest --test-dir build-sanitize --output-on-failure -E hibrido
```

El código usa C11, advertencias estrictas de GCC, `clang-format`, `cppcheck`, AddressSanitizer, UndefinedBehaviorSanitizer y Valgrind durante el desarrollo.
