# HPC-Schelling

Simulación secuencial y paralela híbrida (MPI + OpenMP) de un modelo de segregación residencial basado en Schelling y parametrizado con datos agregados de Montevideo.

## Estado

La especificación funcional está cerrada y están implementados los hitos 1 a 7: simulación secuencial, reproducibilidad, persistencia, índice espacial, distribución MPI, paralelismo OpenMP, automatización experimental y ejecución concurrente de escenarios. Las mediciones en FING se incorporarán cuando se habilite la conexión.

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
ctest --test-dir build --output-on-failure -E '^mpi_'
```

## Ejecución actual

```bash
./build/schelling_seq --config config/base.conf --validate
OMP_NUM_THREADS=2 mpirun -np 2 ./build/schelling_hybrid --config config/base.conf --validate
```

El ejecutable secuencial genera el estado inicial y ejecuta la cantidad configurada de iteraciones. El ejecutable híbrido reparte satisfacción, actualización de precios y búsquedas entre procesos MPI y threads OpenMP; consolida solicitudes globalmente e intercambia halos entre franjas vecinas. Para ejecuciones locales se recomienda fijar afinidad:

```bash
OMP_NUM_THREADS=4 OMP_PROC_BIND=close OMP_PLACES=cores \
    mpirun -np 2 ./build/schelling_hybrid --config config/base.conf --validate
```

La opción `tamanoBloqueVacantes` del archivo de configuración controla el lado de los bloques del índice espacial. El valor base es 32 celdas.

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
ASAN_OPTIONS=detect_leaks=0 ctest --test-dir build-sanitize --output-on-failure -E '^mpi_'
```

El código usa C11, advertencias estrictas de GCC, `clang-format`, `cppcheck`, AddressSanitizer, UndefinedBehaviorSanitizer y Valgrind durante el desarrollo.

## Experimentos locales

La automatización ejecuta un calentamiento y cinco repeticiones por defecto, valida los hashes y genera `resumen.csv` con mediana, dispersión, speedup y eficiencia:

```bash
python3 scripts/ejecutar_experimentos.py --config config/base.conf --iteraciones 1
```

Los resultados crudos se guardan bajo `results/`, que está ignorado por Git. Se puede usar `config/mediana.conf` para una revisión rápida antes de reservar recursos de FING.

Varios escenarios independientes pueden ejecutarse concurrentemente con un límite explícito de trabajos:

```bash
python3 scripts/ejecutar_escenarios.py --trabajos 2 \
    config/mediana.conf config/mediana_sin_ruido.conf config/mediana_sin_permanencia.conf
```

El archivo `resumen_escenarios.csv` combina los resultados sin mezclar los directorios ni las salidas de cada simulación.
