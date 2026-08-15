# HPC-Schelling

Simulación secuencial y paralela híbrida (MPI + OpenMP) de un modelo de segregación residencial basado en Schelling y parametrizado con datos agregados de Montevideo.

## Contenido

El repositorio incluye:

- `app`, `src` e `include`: implementación en C;
- `config`: escenarios de simulación;
- `scripts`: ejecución de campañas y generación de imágenes;
- `tests`: pruebas funcionales;
- `informe`: artículo y datos experimentales utilizados.

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

## Ejecución

Las configuraciones incluidas son:

| Archivo | Uso |
|---|---|
| `base.conf` | escenario funcional de `1024×640` y 120 iteraciones |
| `hpc.conf` | instancia principal de rendimiento de `4096×2560` y 240 iteraciones |
| `mediana.conf` | escenario corto de `256×160` para sensibilidad |
| `mediana_sin_ruido.conf` | escenario mediano sin variación aleatoria de precios |
| `mediana_sin_permanencia.conf` | escenario mediano sin bloqueo posterior a una mudanza |

Cada archivo contiene todos los parámetros necesarios. Las primeras líneas
definen tamaño, iteraciones, vecindario y semilla. Luego aparecen los parámetros
económicos, la proporción de viviendas, la distribución de subestratos y las
tolerancias de las tres clases. `hpc.conf` es una carga sintética y su cantidad
de celdas no representa la población real de Montevideo.

```bash
./build/schelling_seq --config config/base.conf --validate
OMP_NUM_THREADS=2 mpirun -np 2 ./build/schelling_hybrid --config config/base.conf --validate
```

El ejecutable secuencial genera el estado inicial y ejecuta la cantidad configurada de iteraciones. El ejecutable híbrido reparte satisfacción, actualización de precios y búsquedas entre procesos MPI e hilos OpenMP; consolida solicitudes globalmente e intercambia halos entre franjas vecinas. Para ejecuciones locales se recomienda fijar afinidad:

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

El código usa C11 y se compila con advertencias estrictas de GCC.

## Experimentos locales

La automatización ejecuta un calentamiento y cinco repeticiones por defecto, valida los hashes y genera `resumen.csv` con mediana, dispersión, speedup y eficiencia:

```bash
python3 scripts/ejecutar_experimentos.py --config config/base.conf --iteraciones 1
```

En FING se puede indicar el archivo con los nombres de las máquinas:

```bash
python3 scripts/ejecutar_experimentos.py --config config/base.conf \
    --secuencial build-make/schelling_seq \
    --hibrido build-make/schelling_hybrid \
    --salida results/fing --iteraciones 10 --hostfile mis_hosts \
    --configuraciones 1x1,1x2,1x4,2x2,2x4
```

Cada línea de `mis_hosts` debe contener un nombre habilitado por el curso. Conviene realizar primero una ejecución corta con una repetición para estimar cuánto demorará la campaña completa.

La instancia principal de rendimiento se ejecuta sustituyendo la configuración
por `config/hpc.conf`. En FING se debe definir además `FI_PROVIDER=tcp` para las
pcunix utilizadas en este proyecto.

Los resultados crudos se guardan bajo `results/`, que está ignorado por Git. Se puede usar `config/mediana.conf` para una revisión rápida antes de reservar recursos de FING.

Varios escenarios independientes pueden ejecutarse concurrentemente con un límite explícito de trabajos:

```bash
python3 scripts/ejecutar_escenarios.py --trabajos 2 \
    config/mediana.conf config/mediana_sin_ruido.conf config/mediana_sin_permanencia.conf
```

El archivo `resumen_escenarios.csv` combina los resultados sin mezclar los directorios ni las salidas de cada simulación.
# Análisis visual

El estado inicial se puede reconstruir con la misma configuración y semilla. Al terminar una
ejecución, el siguiente comando compara ese estado con el archivo final, calcula la proporción
media de vecinos de la misma clase y crea las dos grillas en formato PPM:

```bash
./build/schelling_analizar --config config/hpc.conf \
  --state results/fing/hpc/estado_final.bin --output results/fing/hpc/grilla
```

Los colores son naranja para clase alta, azul para clase media, verde para clase baja, gris claro
para viviendas vacías y gris oscuro para celdas no residenciales. Las imágenes se convierten a PNG
sin dependencias adicionales con:

```bash
python3 scripts/ppm_a_png.py results/fing/hpc/grilla_inicial.ppm \
  informe/figuras/grilla_hpc_inicial.png
python3 scripts/ppm_a_png.py results/fing/hpc/grilla_final.ppm \
  informe/figuras/grilla_hpc_final.png
```
