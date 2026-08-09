# Informe LaTeX

El informe se divide por secciones para actualizarlo junto con el código. El punto de entrada es `main.tex`.

Compilación recomendada:

```bash
make -C informe
```

El PDF se genera como `informe/informe.pdf` y no se versiona. La compilación requiere `make` y una distribución LaTeX con la clase `IEEEtran`, `latexmk` y `bibtex`.

Para localizar texto incompleto antes de una entrega:

```bash
make -C informe pendientes
```

Las tablas y figuras de resultados deben generarse desde los datos producidos por los experimentos. Los archivos auxiliares generados deben ir en `informe/build/`.
