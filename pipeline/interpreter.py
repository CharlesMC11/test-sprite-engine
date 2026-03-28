import struct
import sys
from typing import Final

import cv2
import numpy as np

from pipeline import (
    SPRITE_HEIGHT,
    SPRITE_METADATA_LAYOUT,
    SPRITE_WIDTH,
    ColorEncoding,
)

SCALE_2BIT_TO_8: Final[int] = 0xFF // 0x03
SCALE_5BIT_TO_8: Final[int] = 0xFF // 0x1F
SCALE_6BIT_TO_8: Final[int] = 0xFF // 0x3F


def unpack_16bit_to_color(
    raw_buffer: bytes, encoding: ColorEncoding
) -> np.ndarray:
    """Unpack the uint16 into 8-bit BGR channels."""

    packed_color = np.frombuffer(raw_buffer, dtype=np.uint16)

    if encoding == ColorEncoding.DEFAULT:
        r = ((packed_color >> 11) & 0x1F) * SCALE_5BIT_TO_8
        g = ((packed_color >> 5) & 0x3F) * SCALE_6BIT_TO_8
        b = (packed_color & 0x1F) * SCALE_5BIT_TO_8

    elif encoding == ColorEncoding.WARM:
        r = (packed_color >> 10 & 0x3F) * SCALE_6BIT_TO_8
        g = (packed_color >> 5 & 0x1F) * SCALE_5BIT_TO_8
        b = (packed_color & 0x1F) * SCALE_5BIT_TO_8

    elif encoding == ColorEncoding.COOL:
        r = (packed_color >> 11 & 0x1F) * SCALE_5BIT_TO_8
        g = (packed_color >> 5 & 0x1F) * SCALE_5BIT_TO_8
        b = (packed_color & 0x3F) * SCALE_6BIT_TO_8

    else:
        raise ValueError("Invalid color mode.")

    return np.stack((b, g, r), axis=-1).astype(np.uint8)


def decompile_sprite(filename: str):
    """Decompile the sprite into a previewable image."""

    with open(f"data/{filename}.sprite", "rb") as f:
        buffer = f.read()

    metadata_buf = buffer[:8]
    palette_buf = buffer[8:40]
    pixels_buf = buffer[40:1064]

    min_x, min_y, max_x, max_y, anchor_x, anchor_y, encoding = struct.unpack(
        f"<{SPRITE_METADATA_LAYOUT}", metadata_buf
    )
    encoding = ColorEncoding(encoding)

    unpacked_palette = unpack_16bit_to_color(palette_buf, encoding)

    pixels = np.frombuffer(pixels_buf, dtype=np.uint8)
    indices = pixels & 0x0F
    alphas = (pixels >> 4) & 0x03

    image_bgr = unpacked_palette[indices].reshape(
        (SPRITE_HEIGHT, SPRITE_WIDTH, 3)
    )
    alpha_mask = alphas.reshape((SPRITE_HEIGHT, SPRITE_WIDTH)) * SCALE_2BIT_TO_8
    alpha_mask = alpha_mask.astype(np.uint8)

    return cv2.merge((image_bgr, alpha_mask))


def main() -> None:
    if len(sys.argv) < 2:
        raise ValueError("Not enough arguments.")

    filename = sys.argv[1]
    preview = decompile_sprite(filename)

    cv2.imshow(
        f"Preview: {filename}",
        cv2.resize(preview, (256, 256), interpolation=cv2.INTER_NEAREST),
    )
    cv2.waitKey(0)


if __name__ == "__main__":
    main()
