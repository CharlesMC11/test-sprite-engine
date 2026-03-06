import struct
import sys

import cv2
import numpy as np

from pipeline.core import (
    HEIGHT,
    WIDTH,
    MAX_PALETTE_SIZE,
    SPRITE_METADATA,
    ColorEncoding,
)


def calculate_hitbox(image: np.ndarray) -> tuple[int, int, int, int]:
    """"""

    if image.shape[2] == 4:
        alpha = image[:, :, 3]
    else:
        alpha = np.ones((HEIGHT, WIDTH)) * 0xFF

    visible_coords = np.argwhere(alpha > 0)
    if visible_coords.size > 0:
        min_y, min_x = visible_coords.min(axis=0)
        max_y, max_x = visible_coords.max(axis=0)
    else:
        min_x = min_y = max_x = max_y = 0

    return min_x, min_y, max_x, max_y


def extract_palette(image: np.ndarray) -> np.ndarray:
    """Extract the palette from an image."""

    pixels = image.reshape(-1, 3)
    unique_colors, counts = np.unique(pixels, axis=0, return_counts=True)
    sorted_indices = np.argsort(-counts)

    return unique_colors[sorted_indices[:MAX_PALETTE_SIZE]]


def pack_color_to_16bit(
    image_bgr: np.ndarray, encoding: ColorEncoding
) -> np.ndarray:
    """Pack the 8-bit BGR channels into an uint16."""

    b, g, r = image_bgr.astype(np.uint16).T

    if encoding == ColorEncoding.DEFAULT:
        packed_color = (r >> 3) << 11 | (g >> 2) << 5 | b >> 3

    elif encoding == ColorEncoding.WARM:
        packed_color = (r >> 2) << 10 | (g >> 3) << 5 | b >> 3

    elif encoding == ColorEncoding.COOL:
        packed_color = (r >> 3) << 11 | (g >> 3) << 5 | b >> 2

    else:
        raise ValueError("Invalid color mode.")

    return packed_color


def compile_sprite(
    output_name: str,
    image: np.ndarray,
    palette: np.ndarray,
    encoding: ColorEncoding,
) -> None:
    """Compile the image into a sprite file."""

    min_x, min_y, max_x, max_y = calculate_hitbox(image)
    anchor_x = (max_x - min_x) // 2
    anchor_y = max_y

    metadata_bytes = struct.pack(
        f"<{SPRITE_METADATA}",
        min_x,
        min_y,
        max_x,
        max_y,
        anchor_x,
        anchor_y,
        encoding.value,
    )

    palette_bytes = bytearray()
    palette_size = len(palette)
    packed_palette = pack_color_to_16bit(palette, encoding)
    for i in range(MAX_PALETTE_SIZE):
        if i < palette_size:
            color = packed_palette[i]
            palette_bytes.extend(struct.pack("<H", color))
        else:
            palette_bytes.extend(struct.pack("<H", 0x00))

    pixels = image[:, :, :3].reshape(-1, 3)
    diffs = pixels[:, np.newaxis, :] - palette[np.newaxis, :, :3]
    distances = np.linalg.norm(diffs, axis=2)
    indices = np.argmin(distances, axis=1)

    if image.shape[2] == 4:
        alphas = image[:, :, 3].flatten() >> 6 & 0x03
    else:
        alphas = np.full(HEIGHT * WIDTH, 0x03, dtype=np.uint8)

    packed_pixels = alphas << 4 | indices & 0x0F
    pixel_bytes = packed_pixels.astype(np.uint8).tobytes()

    padding_bytes = struct.pack("<Q", 0)

    with open(f"data/{output_name}.sprite", "wb") as f:
        f.write(metadata_bytes + palette_bytes + pixel_bytes + padding_bytes)


def main() -> None:
    if len(sys.argv) < 3:
        raise ValueError("Not enough arguments.")

    input_image_path = sys.argv[1]
    output_sprite_name = sys.argv[2]

    try:
        encoding = sys.argv[3].upper()
        if encoding == ColorEncoding.DEFAULT.name:
            encoding = ColorEncoding.DEFAULT
        elif encoding == ColorEncoding.WARM.name:
            encoding = ColorEncoding.WARM
        elif encoding == ColorEncoding.COOL.name:
            encoding = ColorEncoding.COOL
        else:
            raise ValueError("Unknown color encoding.")
    except IndexError:
        encoding = ColorEncoding.DEFAULT

    image = cv2.imread(input_image_path, cv2.IMREAD_UNCHANGED)
    if image is None:
        raise ValueError("Could not read input file.")

    palette = extract_palette(image[:, :, :3])

    compile_sprite(output_sprite_name, image, palette, encoding)


if __name__ == "__main__":
    main()
