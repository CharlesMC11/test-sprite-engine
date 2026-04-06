"""
Asset Interpreter.

Preview a specialized binary format.

The binary layout contains:
- Header (24 bytes):
    - Magic (8 bytes)
    - Sprite metadata (16 bytes)
        - Bounding box (4 bytes)
            - u_min, u_max (2 bytes)
            - v_min, v_max (2 bytes)
        - Anchor (8 bytes)
            - u_anchor (4 bytes)
            - v_anchor (4 bytes)
        - Depth (1 byte)
        - Physics type (1 byte)
        - Color encoding (1 byte)
        - Palette index (1 byte), set to 0x3F (ASCII for '?') as a placeholder
- Pixels (height×width bytes): 1-byte packed values [S][E][AA][IIII]
- Footer (34 bytes): additional metadata
    - Color palette (32 bytes): 16 2-byte unique colors
    - Width (1 byte): width in pixels
    - Height (1 byte): height in pixels
"""

import sys
from pathlib import Path
from typing import Final
from warnings import warn

import cv2
import numpy as np

from pipeline import (
    FOOTER_SIZE_BYTES,
    SPRITE_DIMENSIONS_SIZE_BYTES,
    BGRImage,
    ColorEncoding,
    ResourceLayoutWarning,
    SpriteMetadata,
    is_power_of_2,
)

SCALE_2BIT_TO_8: Final[int] = 0xFF // 0x03
SCALE_5BIT_TO_8: Final[int] = 0xFF // 0x1F
SCALE_6BIT_TO_8: Final[int] = 0xFF // 0x3F


def main() -> None:
    if len(sys.argv) < 2:
        raise ValueError("No filename given.")

    filename = sys.argv[1]
    path = Path(f"assets/{filename}.sprite")
    if not path.exists():
        raise FileNotFoundError(f"Missing asset file: '{path}'")

    image = decompile_asset(path)

    cv2.imshow(
        f"Preview: {filename}",
        cv2.resize(image, (512, 512), interpolation=cv2.INTER_NEAREST),
    )
    cv2.waitKey(0)


def decompile_asset(source_path: Path) -> BGRImage:
    """
    Decompile the asset into a previewable image.

    :param source_path: The path to the asset to decompile.

    :returns: The previewable image.
    """

    buffer = source_path.read_bytes()
    meta = SpriteMetadata.from_bytes(buffer)

    pixels_blob = buffer[
        SpriteMetadata.EXPECTED_SIZE_BYTES : -FOOTER_SIZE_BYTES
    ]
    palette_blob = buffer[-FOOTER_SIZE_BYTES:-SPRITE_DIMENSIONS_SIZE_BYTES]
    width, height = buffer[-SPRITE_DIMENSIONS_SIZE_BYTES:]

    if not (is_power_of_2(width) and is_power_of_2(height)):
        warn(
            f"'{source_path.name}' has invalid dimensions: {width}×{height}!",
            ResourceLayoutWarning,
        )

    pixels = np.frombuffer(pixels_blob, dtype=np.uint8)
    index_bits = pixels & 0x0F
    alpha_bits = (pixels >> 4) & 0x03

    unpacked_palette = _unpack_16bit_to_color(palette_blob, meta.color_encoding)
    image_bgr = unpacked_palette[index_bits].reshape((height, width, 3))

    alpha_mask = alpha_bits.reshape((height, width)) * SCALE_2BIT_TO_8
    alpha_mask = alpha_mask.astype(np.uint8)

    return cv2.merge((image_bgr, alpha_mask))


def _unpack_16bit_to_color(
    packed_buffer: bytes, encoding: ColorEncoding
) -> BGRImage:
    """
    Unpack 2-byte integers into 1-byte BGR channels.

    :param packed_buffer: The buffer containing the 2-byte integers.
    :param encoding: The color encoding to use.

    :returns: The unpacked BGR image.

    :raises ValueError: If the given color encoding is invalid.
    """

    packed_color = np.frombuffer(packed_buffer, dtype=np.uint16)

    if encoding == ColorEncoding.NEUTRAL:
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
        raise ValueError("Invalid color encoding.")

    return np.stack((b, g, r), axis=-1).astype(np.uint8)


if __name__ == "__main__":
    main()
