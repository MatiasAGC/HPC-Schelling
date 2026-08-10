#!/usr/bin/env python3

import argparse
import csv
import json
import os
import platform
import shutil
import statistics
import subprocess
import time
from pathlib import Path


def leerArgumentos():
    parser = argparse.ArgumentParser(description="ejecuta experimentos reproducibles de schelling")
    parser.add_argument("--secuencial", default="build-perf/schelling_seq")
    parser.add_argument("--hibrido", default="build-perf/schelling_hybrid")
    parser.add_argument("--config", default="config/base.conf")
    parser.add_argument("--salida", default="results/local")
    parser.add_argument("--iteraciones", type=int, default=1)
    parser.add_argument("--calentamientos", type=int, default=1)
    parser.add_argument("--repeticiones", type=int, default=5)
    parser.add_argument("--configuraciones", default="1x1,1x2,1x4,2x2,2x4")
    parser.add_argument("--hostfile", default="")
    return parser.parse_args()


def leerTiempoYHash(directorio):
    with (directorio / "timings.csv").open(newline="", encoding="utf-8") as archivo:
        filasTiempos = list(csv.DictReader(archivo))
        tiempos = [float(fila["segundos"]) for fila in filasTiempos]
    with (directorio / "metrics.csv").open(newline="", encoding="utf-8") as archivo:
        metricas = list(csv.DictReader(archivo))
    rutaParalela = directorio / "parallel.csv"
    if rutaParalela.exists():
        with rutaParalela.open(newline="", encoding="utf-8") as archivo:
            paralelas = list(csv.DictReader(archivo))
        desbalance = statistics.mean(float(fila["desbalance"]) for fila in paralelas)
        bytesComunicados = sum(int(fila["bytesComunicados"]) for fila in paralelas)
        solicitudesRemotas = sum(int(fila["solicitudesRemotas"]) for fila in paralelas)
    else:
        desbalance = 1.0
        bytesComunicados = 0
        solicitudesRemotas = 0
    fases = [sum(float(fila[nombre]) for fila in filasTiempos)
             for nombre in ("preparacion", "indices", "busqueda", "comunicacion", "consolidacion")]
    return (sum(tiempos), metricas[-1]["hash"], desbalance, bytesComunicados,
            solicitudesRemotas, *fases)


def ejecutar(comando, entorno, directorio):
    if directorio.exists():
        shutil.rmtree(directorio)
    directorio.parent.mkdir(parents=True, exist_ok=True)
    resultado = subprocess.run(comando + ["--output", str(directorio)], env=entorno,
                               stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    if resultado.returncode != 0:
        print(resultado.stdout, end="")
        raise subprocess.CalledProcessError(resultado.returncode, comando)
    archivosEsperados = (directorio / "timings.csv", directorio / "metrics.csv")
    for _ in range(100):
        if all(archivo.exists() for archivo in archivosEsperados):
            break
        time.sleep(0.1)
    muestra = leerTiempoYHash(directorio)
    estadoFinal = directorio / "estado_final.bin"
    if estadoFinal.exists():
        estadoFinal.unlink()
    return muestra


def resumir(nombre, procesos, threads, muestras, tiempoBase, hashEsperado):
    tiempos = [muestra[0] for muestra in muestras]
    hashes = {muestra[1] for muestra in muestras}
    if hashes != {hashEsperado}:
        raise RuntimeError(f"hash inconsistente en {nombre}: {sorted(hashes)}")
    mediana = statistics.median(tiempos)
    recursos = procesos * threads
    speedup = tiempoBase / mediana
    return {
        "configuracion": nombre,
        "procesos": procesos,
        "threads": threads,
        "recursos": recursos,
        "repeticiones": len(tiempos),
        "medianaSegundos": f"{mediana:.9f}",
        "minimoSegundos": f"{min(tiempos):.9f}",
        "maximoSegundos": f"{max(tiempos):.9f}",
        "desviacionSegundos": f"{statistics.pstdev(tiempos):.9f}",
        "speedup": f"{speedup:.6f}",
        "eficiencia": f"{speedup / recursos:.6f}",
        "desbalanceMedio": f"{statistics.median(muestra[2] for muestra in muestras):.6f}",
        "bytesComunicados": str(int(statistics.median(muestra[3] for muestra in muestras))),
        "solicitudesRemotas": str(int(statistics.median(muestra[4] for muestra in muestras))),
        "preparacionSegundos": f"{statistics.median(muestra[5] for muestra in muestras):.9f}",
        "indicesSegundos": f"{statistics.median(muestra[6] for muestra in muestras):.9f}",
        "busquedaSegundos": f"{statistics.median(muestra[7] for muestra in muestras):.9f}",
        "comunicacionSegundos": f"{statistics.median(muestra[8] for muestra in muestras):.9f}",
        "consolidacionSegundos": f"{statistics.median(muestra[9] for muestra in muestras):.9f}",
        "hash": hashEsperado,
    }


def main():
    argumentos = leerArgumentos()
    salida = Path(argumentos.salida)
    entornoBase = os.environ.copy()
    entornoBase["OMP_PROC_BIND"] = "close"
    entornoBase["OMP_PLACES"] = "cores"
    comun = ["--config", argumentos.config, "--iterations", str(argumentos.iteraciones),
             "--checkpoint-every", "0", "--validate"]

    configuraciones = []
    for valor in argumentos.configuraciones.split(","):
        procesos, threads = (int(numero) for numero in valor.lower().split("x"))
        configuraciones.append((procesos, threads))

    ejecuciones = [("secuencial", 1, 1, [argumentos.secuencial] + comun)]
    for procesos, threads in configuraciones:
        comando = ["mpirun", "-np", str(procesos)]
        if argumentos.hostfile:
            comando.extend(["-hostfile", argumentos.hostfile])
        comando.extend([argumentos.hibrido] + comun)
        ejecuciones.append((f"mpi{procesos}_omp{threads}", procesos, threads, comando))

    resultados = {}
    hashEsperado = None
    for nombre, procesos, threads, comando in ejecuciones:
        entorno = entornoBase.copy()
        entorno["OMP_NUM_THREADS"] = str(threads)
        for calentamiento in range(argumentos.calentamientos):
            ejecutar(comando, entorno, salida / nombre / f"calentamiento_{calentamiento + 1}")
        muestras = []
        for repeticion in range(argumentos.repeticiones):
            muestras.append(ejecutar(comando, entorno,
                                     salida / nombre / f"repeticion_{repeticion + 1}"))
        if hashEsperado is None:
            hashEsperado = muestras[0][1]
        resultados[nombre] = (procesos, threads, muestras)

    tiempoBase = statistics.median([muestra[0] for muestra in resultados["secuencial"][2]])
    filas = [resumir(nombre, procesos, threads, muestras, tiempoBase, hashEsperado)
             for nombre, (procesos, threads, muestras) in resultados.items()]
    salida.mkdir(parents=True, exist_ok=True)
    with (salida / "resumen.csv").open("w", newline="", encoding="utf-8") as archivo:
        escritor = csv.DictWriter(archivo, fieldnames=filas[0].keys())
        escritor.writeheader()
        escritor.writerows(filas)
    metadatos = vars(argumentos).copy()
    metadatos["plataforma"] = platform.platform()
    metadatos["procesador"] = platform.processor()
    metadatos["python"] = platform.python_version()
    metadatos["host"] = platform.node()
    with (salida / "experimento.json").open("w", encoding="utf-8") as archivo:
        json.dump(metadatos, archivo, indent=2, sort_keys=True)
        archivo.write("\n")


if __name__ == "__main__":
    main()
