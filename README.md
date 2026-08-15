# HPC-Schelling

Simulación secuencial y paralela híbrida (MPI + OpenMP) de un modelo de segregación residencial basado en Schelling y configurado con datos generales de Montevideo.

## Contenido

El repositorio incluye:

- `app`, `src` e `include`: implementación en C;
- `config`: archivos con los parámetros de las simulaciones.

## Compilación

La construcción principal utiliza CMake:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF
cmake --build build --parallel
```

También hay un Makefile independiente para los equipos donde CMake no esté disponible:

```bash
make release
```

## Ejecución

Las configuraciones incluidas son:

| Archivo | Uso |
|---|---|
| `base.conf` | grilla preliminar de `1024×640` y 120 iteraciones |
| `hpc.conf` | grilla principal de `4096×2560` y 240 iteraciones |
| `mediana.conf` | grilla corta de `256×160` para comparar parámetros |
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

El ejecutable secuencial genera el estado inicial y ejecuta la cantidad configurada de iteraciones. El ejecutable híbrido reparte satisfacción, actualización de precios y búsquedas entre procesos MPI e hilos OpenMP; consolida solicitudes globalmente e intercambia filas auxiliares entre franjas vecinas. Para ejecuciones locales se recomienda fijar afinidad:

```bash
OMP_NUM_THREADS=4 OMP_PROC_BIND=close OMP_PLACES=cores \
    mpirun -np 2 ./build/schelling_hybrid --config config/base.conf --validate
```

La opción `tamanoBloqueVacantes` del archivo de configuración controla el lado de los bloques del índice espacial. El valor base es 32 celdas.

Para consultar todas las opciones:

```bash
./build/schelling_seq --help
```

## Archivos generados

Cada ejecución crea los archivos `run.json`, `metrics.csv`, `timings.csv` y
`estado_final.bin` en el directorio indicado mediante `--output`. La versión
híbrida agrega `parallel.csv` con datos sobre comunicación y distribución del
trabajo.

## Análisis visual

El estado inicial se puede reconstruir con la misma configuración y semilla. Al terminar una
ejecución, el siguiente comando compara ese estado con el archivo final, calcula la proporción
media de vecinos de la misma clase y crea las dos grillas en formato PPM:

```bash
./build/schelling_analizar --config config/hpc.conf \
  --state salida/estado_final.bin --output salida/grilla
```

Los colores son naranja para clase alta, azul para clase media, verde para clase baja, gris claro
para viviendas vacías y gris oscuro para celdas no residenciales. Las grillas se guardan en
formato PPM y pueden abrirse con un visor de imágenes compatible.
