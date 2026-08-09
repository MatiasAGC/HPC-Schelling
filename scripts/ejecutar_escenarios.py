#!/usr/bin/env python3

import argparse
import csv
import subprocess
from concurrent.futures import ThreadPoolExecutor, as_completed
from pathlib import Path


def leerArgumentos():
    parser = argparse.ArgumentParser(description="ejecuta varios escenarios en paralelo")
    parser.add_argument("escenarios", nargs="+", help="archivos de configuracion")
    parser.add_argument("--salida", default="results/escenarios")
    parser.add_argument("--trabajos", type=int, default=2)
    parser.add_argument("--secuencial", default="build-perf/schelling_seq")
    parser.add_argument("--hibrido", default="build-perf/schelling_hybrid")
    parser.add_argument("--iteraciones", type=int, default=10)
    parser.add_argument("--calentamientos", type=int, default=1)
    parser.add_argument("--repeticiones", type=int, default=5)
    parser.add_argument("--configuraciones", default="1x1,1x4,2x4")
    argumentos = parser.parse_args()
    if argumentos.trabajos <= 0:
        parser.error("trabajos debe ser positivo")
    return argumentos


def ejecutarEscenario(argumentos, escenario):
    nombre = escenario.stem
    directorio = Path(argumentos.salida) / nombre
    comando = [
        "python3",
        str(Path(__file__).with_name("ejecutar_experimentos.py")),
        "--secuencial", argumentos.secuencial,
        "--hibrido", argumentos.hibrido,
        "--config", str(escenario),
        "--salida", str(directorio),
        "--iteraciones", str(argumentos.iteraciones),
        "--calentamientos", str(argumentos.calentamientos),
        "--repeticiones", str(argumentos.repeticiones),
        "--configuraciones", argumentos.configuraciones,
    ]
    resultado = subprocess.run(comando, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                               text=True)
    if resultado.returncode != 0:
        raise RuntimeError(f"fallo el escenario {escenario}\n{resultado.stdout}")
    return nombre, directorio / "resumen.csv"


def combinarResultados(resultados, salida):
    filas = []
    campos = None
    for escenario, ruta in sorted(resultados):
        with ruta.open(newline="", encoding="utf-8") as archivo:
            lector = csv.DictReader(archivo)
            if campos is None:
                campos = ["escenario"] + lector.fieldnames
            for fila in lector:
                filas.append({"escenario": escenario, **fila})

    salida.mkdir(parents=True, exist_ok=True)
    with (salida / "resumen_escenarios.csv").open("w", newline="", encoding="utf-8") as archivo:
        escritor = csv.DictWriter(archivo, fieldnames=campos)
        escritor.writeheader()
        escritor.writerows(filas)


def main():
    argumentos = leerArgumentos()
    escenarios = [Path(ruta) for ruta in argumentos.escenarios]
    inexistentes = [str(ruta) for ruta in escenarios if not ruta.is_file()]
    if inexistentes:
        raise FileNotFoundError(f"escenarios inexistentes: {', '.join(inexistentes)}")
    nombres = [escenario.stem for escenario in escenarios]
    if len(nombres) != len(set(nombres)):
        raise ValueError("los escenarios deben tener nombres de archivo unicos")

    resultados = []
    with ThreadPoolExecutor(max_workers=argumentos.trabajos) as ejecutor:
        futuros = [ejecutor.submit(ejecutarEscenario, argumentos, escenario)
                   for escenario in escenarios]
        for futuro in as_completed(futuros):
            resultados.append(futuro.result())
    combinarResultados(resultados, Path(argumentos.salida))


if __name__ == "__main__":
    main()
