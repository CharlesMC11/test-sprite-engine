import struct
import sys
from enum import IntEnum
import enum

import cv2
import numpy as np

HEIGHT: int = 32
WIDTH: int = 32
MAX_PALETTE_SIZE: int = 16


class ColorMode(IntEnum):
    DEFAULT = 1  # R5G6B5
    WARM = enum.auto()  # R6G5B5
    COOL = enum.auto()  # R5G5B6
    FOREST = enum.auto()  # R3G10B3
    SEA = enum.auto()  # R2G2B12


def get_palette(img: np.ndarray) -> np.ndarray:
    """Get the palette from an image."""

    pixels = img.reshape(-1, 3)
    unique_colors, counts = np.unique(pixels, axis=0, return_counts=True)
    sorted_indices = np.argsort(-counts)

    return unique_colors[sorted_indices[:MAX_PALETTE_SIZE]]


def pack_pixel_bytes(pixels: np.ndarray, mode: ColorMode) -> int:
    result = 0x0

    b, g, r = pixels.astype(np.uint16).T

    if mode == ColorMode.DEFAULT:
        result = (r >> 3) << 11 | (g >> 2) << 5 | b >> 3

    elif mode == ColorMode.WARM:
        result = (r >> 2) << 10 | (g >> 3) << 5 | b >> 3

    elif mode == ColorMode.COOL:
        result = (r >> 3) << 11 | (g >> 3) << 5 | b >> 2

    elif mode == ColorMode.FOREST:
        g = (g / 0xFF * 0x3FF).astype(np.uint16)
        result = (r >> 5) << 13 | g << 3 | b >> 5

    elif mode == ColorMode.SEA:
        b = (b / 0xFF * 0xFFF).astype(np.uint16)
        result = (r >> 6) << 14 | (g >> 6) << 12 | b

    return result


def bake(
        img: np.ndarray, palette: np.ndarray, mode: ColorMode, output_name: str
) -> None:
    header = struct.pack("<8s B B B 21x", b"SPRITE", mode.value, 0, 0)

    palette_bytes = bytearray()
    palette_size = len(palette)
    normalized_palette = pack_pixel_bytes(palette, mode)
    for i in range(MAX_PALETTE_SIZE):
        if i < palette_size:
            color = normalized_palette[i]
            palette_bytes.extend(struct.pack("<H", color))
        else:
            palette_bytes.extend((struct.pack("<H", 0x0)))

    pixel_bytes = bytearray()
    for y in range(HEIGHT):
        for x in range(WIDTH):
            pixel = img[y, x]
            alpha = (pixel[3] >> 6) << 4 if len(pixel) == 4 else 0x30
            dists = np.linalg.norm(palette[:, :3] - pixel[:3], axis=1)
            idx = np.argmin(dists).astype(np.uint8) & 0x0F
            pixel_bytes.append(alpha | idx)

    with open(f"data/{output_name}.sprite", "wb") as f:
        f.write(header + palette_bytes + pixel_bytes)


def main() -> None:
    if len(sys.argv) < 2:
        sys.exit(1)

    in_path = sys.argv[1]

    mode = sys.argv[2].upper()
    if mode == ColorMode.DEFAULT.name:
        mode = ColorMode.DEFAULT
    elif mode == ColorMode.WARM.name:
        mode = ColorMode.WARM
    elif mode == ColorMode.COOL.name:
        mode = ColorMode.COOL
    elif mode == ColorMode.FOREST.name:
        mode = ColorMode.FOREST
    elif mode == ColorMode.SEA.name:
        mode = ColorMode.SEA
    else:
        raise ValueError("Unknown mode.")

    out_path = sys.argv[3]

    img = cv2.imread(in_path, cv2.IMREAD_UNCHANGED)
    if img is None:
        raise ValueError("Could not read input file.")

    palette = get_palette(img[:, :, :3])

    bake(img, palette, mode, out_path)


if __name__ == "__main__":
    main()
