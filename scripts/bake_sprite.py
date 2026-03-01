import enum
import struct
import sys
from enum import IntEnum

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


def extract_palette(img: np.ndarray) -> np.ndarray:
    """Extract the palette from an image."""

    pixels = img.reshape(-1, 3)
    unique_colors, counts = np.unique(pixels, axis=0, return_counts=True)
    sorted_indices = np.argsort(-counts)

    return unique_colors[sorted_indices[:MAX_PALETTE_SIZE]]


def pack_color_to_16bit(bgr: np.ndarray, mode: ColorMode) -> np.ndarray:
    """Pack the color channels into a 16-bit unsigned int."""

    b, g, r = bgr.astype(np.uint16).T

    if mode == ColorMode.DEFAULT:
        packed_color = (r >> 3) << 11 | (g >> 2) << 5 | b >> 3

    elif mode == ColorMode.WARM:
        packed_color = (r >> 2) << 10 | (g >> 3) << 5 | b >> 3

    elif mode == ColorMode.COOL:
        packed_color = (r >> 3) << 11 | (g >> 3) << 5 | b >> 2

    elif mode == ColorMode.FOREST:
        g = (g / 0xFF * 0x3FF).astype(np.uint16)
        packed_color = (r >> 5) << 13 | g << 3 | b >> 5

    elif mode == ColorMode.SEA:
        b = (b / 0xFF * 0xFFF).astype(np.uint16)
        packed_color = (r >> 6) << 14 | (g >> 6) << 12 | b

    else:
        raise ValueError("Invalid color matrix.")

    return packed_color


def bake(
        image: np.ndarray, palette: np.ndarray, mode: ColorMode, out_name: str
) -> None:
    """Bake the data into a custom sprite file."""

    name_bytes = out_name.encode()[:MAX_NAME_BUF]
    header = struct.pack(
        "<8s 16s B B B 5x", b"SPRITE", name_bytes, mode.value, 15, 29
    )

    palette_bytes = bytearray()
    palette_size = len(palette)
    packed_palette = pack_color_to_16bit(palette, mode)
    for i in range(MAX_PALETTE_SIZE):
        if i < palette_size:
            color = packed_palette[i]
            palette_bytes.extend(struct.pack("<H", color))
        else:
            palette_bytes.extend(struct.pack("<H", 0x0))

    pixel_bytes = bytearray()
    for y in range(HEIGHT):
        for x in range(WIDTH):
            pixel = image[y, x]
            alpha = (pixel[3] >> 6) << 4 if len(pixel) == 4 else 0x30
            color_distances = np.linalg.norm(palette[:, :3] - pixel[:3], axis=1)
            index = np.argmin(color_distances).astype(np.uint8) & 0x0F
            pixel_bytes.append(alpha | index)

    with open(f"data/{out_name}.sprite", "wb") as f:
        f.write(header + palette_bytes + pixel_bytes)


def main() -> None:
    if len(sys.argv) < 3:
        raise ValueError("Not enough arguments.")

    input_image_path = sys.argv[1]
    output_sprite_name = sys.argv[2]

    try:
        mode = sys.argv[3].upper()
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
            raise ValueError("Unknown color mode.")
    except IndexError:
        mode = ColorMode.DEFAULT

    image = cv2.imread(input_image_path, cv2.IMREAD_UNCHANGED)
    if image is None:
        raise ValueError("Could not read input file.")

    palette = extract_palette(image[:, :, :3])

    bake(image, palette, mode, output_sprite_name)


if __name__ == "__main__":
    main()
