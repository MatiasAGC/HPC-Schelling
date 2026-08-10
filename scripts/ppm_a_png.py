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


def convertir(origen, destino):
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
    argumentos = parser.parse_args()
    convertir(argumentos.origen, argumentos.destino)


if __name__ == "__main__":
    main()
