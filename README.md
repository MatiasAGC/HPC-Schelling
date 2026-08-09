# HPC-Schelling

Simulación secuencial y paralela híbrida (MPI + OpenMP) de un modelo de segregación residencial basado en Schelling y parametrizado con datos agregados de Montevideo.

## Estado

La especificación funcional está cerrada y la implementación todavía no comenzó. Los documentos principales son:

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

## Implementación prevista

El proyecto utilizará C11, GCC, CMake, un Makefile portable, MPICH/Open MPI y OpenMP. Cuando exista código, este README incluirá los comandos completos de compilación, pruebas y ejecución.
