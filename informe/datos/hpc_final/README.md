# Respaldo de la campaña principal

Estos archivos conservan los valores de la campaña ejecutada en FING con
`config/hpc.conf`. Los finales de línea de `resumen.csv` se normalizaron para
Git. `grilla_metricas.csv` fue generado después a partir del estado final.

- `resumen.csv` contiene tiempos, speedup, eficiencia, comunicación y hash.
- Cada configuración conserva `run.json`, `metrics.csv`, `timings.csv` y, en
  los casos MPI, `parallel.csv`.
- `experimento.json` describe la campaña.
- `grilla_metricas.csv` compara el agrupamiento inicial y final.

El archivo `estado_final.bin` no se versiona porque ocupa 425 MB y supera el
límite habitual de GitHub para un solo archivo. Su suma SHA-256 es:

```text
f8950696755def075680073606c0bf697261a86f26499662547e4fcdfd882b63
```

El estado se puede reconstruir ejecutando `config/hpc.conf` con la semilla 42.
Debe terminar en la iteración 240 con el hash de modelo
`6b39928d840ed346`. Las imágenes PNG derivadas se guardan en
`informe/figuras`.
