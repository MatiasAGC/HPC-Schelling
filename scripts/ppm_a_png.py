#!/usr/bin/env python3

import argparse
import struct
import zlib
from pathlib import Path


def leer_token(archivo):
    token = bytearray()

    while True:
        caracter = archivo.read(1)

        if not caracter:
            raise ValueError("cabecera ppm incompleta")

        if caracter == b"#":
            archivo.readline()
        elif not caracter.isspace():
            token.extend(caracter)
            break

    while True:
        caracter = archivo.read(1)

        if not caracter or caracter.isspace():
            return bytes(token)

        token.extend(caracter)


def bloque(tipo, datos):
    return (
        struct.pack(">I", len(datos))
        + tipo
        + datos
        + struct.pack(">I", zlib.crc32(tipo + datos))
    )


def recortar(datos, ancho, alto, recorte):
    if recorte is None:
        return datos, ancho, alto

    origen_x, origen_y, nuevo_ancho, nuevo_alto = recorte

    if (
        origen_x < 0
        or origen_y < 0
        or nuevo_ancho <= 0
        or nuevo_alto <= 0
        or origen_x + nuevo_ancho > ancho
        or origen_y + nuevo_alto > alto
    ):
        raise ValueError("el recorte queda fuera de la imagen")

    filas = []

    for fila in range(origen_y, origen_y + nuevo_alto):
        inicio = (fila * ancho + origen_x) * 3
        filas.append(datos[inicio : inicio + nuevo_ancho * 3])

    return b"".join(filas), nuevo_ancho, nuevo_alto


def convertir(origen, destino, recorte=None):
    with origen.open("rb") as archivo:
        if leer_token(archivo) != b"P6":
            raise ValueError("la imagen no usa el formato ppm binario")

        ancho = int(leer_token(archivo))
        alto = int(leer_token(archivo))

        if int(leer_token(archivo)) != 255:
            raise ValueError("la imagen no usa colores de 8 bits")

        datos = archivo.read()

    if len(datos) != ancho * alto * 3:
        raise ValueError("cantidad de pixeles incorrecta")

    datos, ancho, alto = recortar(datos, ancho, alto, recorte)

    filas = b"".join(
        b"\0" + datos[fila * ancho * 3 : (fila + 1) * ancho * 3]
        for fila in range(alto)
    )
    cabecera = struct.pack(">IIBBBBB", ancho, alto, 8, 2, 0, 0, 0)
    contenido = b"\x89PNG\r\n\x1a\n"
    contenido += bloque(b"IHDR", cabecera)
    contenido += bloque(b"IDAT", zlib.compress(filas, 9))
    contenido += bloque(b"IEND", b"")
    destino.write_bytes(contenido)


def main():
    parser = argparse.ArgumentParser(description="convierte una grilla ppm a png")
    parser.add_argument("origen", type=Path)
    parser.add_argument("destino", type=Path)
    parser.add_argument(
        "--crop",
        nargs=4,
        type=int,
        metavar=("X", "Y", "ANCHO", "ALTO"),
        help="recorta la imagen antes de convertirla",
    )
    argumentos = parser.parse_args()
    convertir(argumentos.origen, argumentos.destino, argumentos.crop)


if __name__ == "__main__":
    main()
