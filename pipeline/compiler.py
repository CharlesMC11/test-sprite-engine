"""
Sprite Compiler.

Bakes a 32×32 source (BGR/RGBA) image and optional glow masks into a specialized
1072-byte binary format.

The binary layout contains:
- Metadata (8 bytes): Hitbox and anchor data.
- Palette (32 bytes): 16 colors as 16-bit packed integers.
- Pixels (1,024 bytes): 8-bit packed values [R][G][AA][IIII].
- Padding (8 bytes): For 16-byte alignment.
"""

import struct
from argparse import ArgumentParser
from pathlib import Path

import cv2
import numpy as np
import numpy.typing as npt

from pipeline import (
    MAX_PALETTE_SIZE,
    SPRITE_HEIGHT,
    SPRITE_METADATA,
    SPRITE_WIDTH,
    ColorEncoding,
)

type BGRImage = npt.NDArray[np.uint8]
type AlphaMask = npt.NDArray[np.uint8]
type GlowMask = npt.NDArray[np.uint8]
type Palette = npt.NDArray[np.uint8]

type PackedColors = npt.NDArray[np.uint16]
type BakedPixels = npt.NDArray[np.uint8]


def calculate_hitbox(mask: AlphaMask) -> tuple[int, int, int, int]:
    """
    Calculate the hitbox of a sprite based from an alpha mask.

    :param mask: An alpha mask to calculate the hitbox from.
    :returns: The top-left and bottom-right coordinates of the hitbox.
    """

    visible_coords = np.argwhere(mask > 0)
    if visible_coords.size > 0:
        min_y, min_x = visible_coords.min(axis=0)
        max_y, max_x = visible_coords.max(axis=0)
    else:
        min_x = min_y = max_x = max_y = 0

    return min_x, min_y, max_x, max_y


def pack_colors_to_16bit(
    image_bgr: BGRImage, encoding: ColorEncoding
) -> PackedColors:
    """
    Pack the 8-bit BGR channels into 16-bit integers.

    :param image_bgr: The original color channels to pack.
    :param encoding: The color encoding mode to use.
    :returns: The packed 16-bit colors.
    """

    b, g, r = image_bgr.astype(np.uint16).T

    if encoding == ColorEncoding.DEFAULT:
        packed_colors = (r >> 3) << 11 | (g >> 2) << 5 | b >> 3

    elif encoding == ColorEncoding.WARM:
        packed_colors = (r >> 2) << 10 | (g >> 3) << 5 | b >> 3

    elif encoding == ColorEncoding.COOL:
        packed_colors = (r >> 3) << 11 | (g >> 3) << 5 | b >> 2

    else:
        raise ValueError("Invalid color mode.")

    return packed_colors


class SpriteCompiler:
    """Compiles a source image into a sprite file."""

    # Type annotations

    _encoding: ColorEncoding
    _source_image: BGRImage | None
    _source_alpha: AlphaMask | None
    _pixel_flat_list: BGRImage | None
    _glow_bits: GlowMask | None
    _palette: Palette | None

    # Magic methods
    def __init__(self, encoding: ColorEncoding):
        self._encoding = encoding
        self._source_image = None
        self._source_alpha = None
        self._pixel_flat_list = None
        self._glow_bits = None
        self._palette = None

    # Public methods

    def ingest_asset(
        self, image_path: str, glow_mask_path: Path | None
    ) -> None:
        """
        Validate the source asset.

        Split the source image into color and alpha channels, flatten the pixel
        grid for evaluation, and pre-threshold the glow mask.

        :param image_path: The filesystem path to a 32×32 BGR/RGBA image.
        :param glow_mask_path: Optional path to a 32×32 grayscale glow mask.

        :raises RuntimeError: If OpenCV fails to read the assets.
        :raises ValueError: If the image dimensions are not 32×32.
        """

        if (image := cv2.imread(image_path, cv2.IMREAD_UNCHANGED)) is None:
            raise RuntimeError("Could not read source image.")

        h, w = image.shape[:2]
        if h != SPRITE_HEIGHT or w != SPRITE_WIDTH:
            raise ValueError(
                f"Asset dimensions must be {SPRITE_WIDTH}×{SPRITE_HEIGHT}."
            )

        if image.shape[2] >= 4:
            self._source_image = image[:, :, :3]
            self._source_alpha = image[:, :, 3]
        else:
            self._source_image = image
            self._source_alpha = np.full(
                (SPRITE_HEIGHT, SPRITE_WIDTH), 0xFF, dtype=np.uint8
            )

        self._pixel_flat_list = self._source_image.reshape(-1, 3)

        if glow_mask_path:
            self._ingest_glow_mask(glow_mask_path)
        if self._glow_bits is None:
            self._glow_bits = np.zeros(
                SPRITE_HEIGHT * SPRITE_WIDTH, dtype=np.uint8
            )

        self._palette = self._extract_palette()

    def compile(self, output_path: Path) -> None:
        """
        Compile the image into a sprite file.

        :param output_path: The path to save the sprite to.
        """

        hitbox = calculate_hitbox(self._source_alpha)
        hb_min_x, hb_min_y, hb_max_x, hb_max_y = hitbox
        anchor_x = (hb_max_x - hb_min_x) // 2
        anchor_y = hb_max_y

        metadata_bytes = struct.pack(
            f"<{SPRITE_METADATA}",
            hb_min_x,
            hb_min_y,
            hb_max_x,
            hb_max_y,
            anchor_x,
            anchor_y,
            self._encoding.value,
        )

        palette_bytes = bytearray()
        palette_size = len(self._palette)
        packed_palette = pack_colors_to_16bit(self._palette, self._encoding)
        for i in range(MAX_PALETTE_SIZE):
            if i < palette_size:
                color = packed_palette[i]
                palette_bytes.extend(struct.pack("<H", color))
            else:
                palette_bytes.extend(struct.pack("<H", 0x00))

        pixel_bytes = self._bake_pixels().tobytes()

        padding_bytes = struct.pack("<Q", 0)

        with output_path.open("wb") as f:
            f.write(
                metadata_bytes + palette_bytes + pixel_bytes + padding_bytes
            )

    # Protected methods

    def _ingest_glow_mask(self, glow_mask_path: Path) -> None:
        """
        Ingest the glow mask.

        :param glow_mask_path: The path to the glow mask.
        :raises FileNotFoundError: If the glow mask is missing.
        :raises RuntimeError: If OpenCV fails to read the mask.
        :raises ValueError: If the mask dimensions are not 32×32.
        """

        if not glow_mask_path.exists():
            raise FileNotFoundError(f"Missing glow map file: {glow_mask_path}")
        if (
            glow_mask := cv2.imread(glow_mask_path, cv2.IMREAD_GRAYSCALE)
        ) is None:
            raise RuntimeError(f"Could not read glow mask: {glow_mask_path}.")

        h, w = glow_mask[:2]
        if h != SPRITE_HEIGHT or w != SPRITE_WIDTH:
            raise ValueError(
                f"Mask dimensions must be {SPRITE_WIDTH}×{SPRITE_HEIGHT}."
            )

        self._glow_bits = (glow_mask.flatten() > 0x80).astype(np.uint8)

    def _extract_palette(self) -> Palette:
        """
        Extract 16 unique colors from the source image.

        :returns: The 16 unique colors that were extracted.
        """

        unique_colors, counts = np.unique(
            self._pixel_flat_list, axis=0, return_counts=True
        )
        sorted_indices = np.argsort(-counts)

        return unique_colors[sorted_indices[:MAX_PALETTE_SIZE]]

    def _index_colors(self):
        """
        Calculate the color indices for the extracted palette.

        :returns: The indices to colors in the palette.
        """

        color_deltas = self._pixel_flat_list[:, np.newaxis, :] - self._palette
        distances = np.linalg.norm(color_deltas, axis=2)

        return np.argmin(distances, axis=1)

    def _bake_pixels(self) -> BakedPixels:
        """
        Bake the pixel surface into a final 8-bit format.

        The Bit Mapping:
        - [7] Reserved: Always 0.
        - [6] Glow: Boolean signal (0 or 1).
        - [4–5] Alpha: 2-bit transparency (0–3).
        - [0–3] Palette Index: Pointer to one of the 16 colors.

        :returns: The baked pixels.
        """

        indices = self._index_colors() & 0x0F
        alphas = (self._source_alpha.flatten() >> 6) & 0x03
        baked_pixels = self._glow_bits << 6 | alphas << 4 | indices

        return baked_pixels.astype(np.uint8)


def main() -> None:
    parser = ArgumentParser("Sprite Compiler", description=__doc__)
    parser.add_argument(
        "source_image", help="Path to the source BGR/RGBA image."
    )
    parser.add_argument(
        "output_path", type=Path, help="Target path for the 1072-byte sprite."
    )
    parser.add_argument(
        "-e",
        "--encoding",
        default=ColorEncoding.DEFAULT,
        type=lambda e: ColorEncoding[e.upper()],
        choices=[e.name for e in ColorEncoding],
        help="Color encoding (Default, Warm, or Cool)",
    )
    parser.add_argument(
        "-g",
        "--glow_mask",
        default=None,
        type=Path,
        help="Optional grayscale mask for glow mapping.",
    )

    args = parser.parse_args()

    compiler = SpriteCompiler(args.encoding)
    compiler.ingest_asset(args.source_image, args.glow_mask)
    compiler.compile(args.output_path)


if __name__ == "__main__":
    main()
