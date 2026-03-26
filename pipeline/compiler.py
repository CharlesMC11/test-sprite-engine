"""
Sprite Compiler.

Bakes a 32×32 source (BGR/RGBA) image and optional emission & specular masks
into a specialized 1,072-byte binary format.

The binary layout contains:
- Metadata (16 bytes):
    - Bounding box (4 bytes)
    - Anchor points (8 bytes)
    - Color encoding (1 byte)
    - Palette index (1 byte), set to 0 as a placeholder
    - Physics type (1 byte)
    - Padding (1 byte)
- Pixels (1,024 bytes): 8-bit packed values [S][E][AA][IIII].
- Color palette (32 bytes): 16 unique colors in the sprite.
"""

import struct
from argparse import ArgumentParser
from pathlib import Path
from collections.abc import Sequence

import cv2
import numpy as np
import numpy.typing as npt

from pipeline import (
    MAX_PALETTE_SIZE,
    SPRITE_HEIGHT,
    SPRITE_METADATA_LAYOUT,
    SPRITE_SIZE_BYTES,
    SPRITE_WIDTH,
    ColorEncoding,
    ResourceLayoutError,
    PhysicsType,
)

type BGRImage = npt.NDArray[np.uint8]
type AlphaMask = npt.NDArray[np.uint8]
type EmissionMask = npt.NDArray[np.uint8]
type SpecularMask = npt.NDArray[np.uint8]
type Palette = npt.NDArray[np.uint8]

type PackedColors = npt.NDArray[np.uint16]
type BakedPixels = npt.NDArray[np.uint8]

DEFAULT_SPRITE_AX: Final[float] = SPRITE_WIDTH / 2.0
"""The default horizontal anchor point in sub-pixels."""

DEFAULT_SPRITE_AY: Final[float] = SPRITE_HEIGHT / 2.0
"""The default vertical anchor point in sub-pixels."""


class ResourceLayoutWarning(UserWarning): ...


class SpriteCompiler:
    """Compiles a source image into a sprite file."""

    # Type annotations

    _source_path: Path
    _anchor_x: float
    _anchor_y: float
    _encoding: ColorEncoding
    _physics: PhysicsType
    _source_image: BGRImage | None
    _source_alpha: AlphaMask | None
    _pixel_flat_list: BGRImage | None
    _emission_bits: EmissionMask | None
    _specular_bits: SpecularMask | None
    _palette: Palette | None

    # Magic methods

    def __init__(
        self,
        source_path: Path,
        anchors: Sequence[float],
        encoding: ColorEncoding,
        physics: PhysicsType,
    ):
        self._source_path = source_path
        self._anchor_x, self._anchor_y = anchors
        self._encoding = encoding
        self._physics = physics
        self._source_image = None
        self._source_alpha = None
        self._pixel_flat_list = None
        self._emission_bits = None
        self._specular_bits = None
        self._palette = None

    # Public methods

    def ingest_asset(
        self,
        emission_mask_path: Path | None,
        specular_mask_path: Path | None,
    ) -> None:
        """
        Validate the source asset.

        Split the source image into color and alpha channels, flatten the pixel
        grid for evaluation, and pre-threshold the glow mask.

        :param image_path: The filesystem path to a 32×32 BGR/RGBA image.
        :param emission_mask_path: Optional path to a 32×32 grayscale emission mask.
        :param specular_mask_path: Optional path to a 32×32 grayscale specular mask.

        :raises RuntimeError: If OpenCV fails to read the assets.
        :raises ValueError: If the image dimensions are not 32×32.
        """
        if not self._source_path.exists():
            raise FileNotFoundError(f"Missing image file: {self._source_path}")

        if (
            image := cv2.imread(self._source_path, cv2.IMREAD_UNCHANGED)
        ) is None:
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

        if emission_mask_path:
            self._emission_bits = self._ingest_grayscale_mask(
                emission_mask_path
            )
        if self._emission_bits is None:
            self._emission_bits = np.zeros(
                SPRITE_HEIGHT * SPRITE_WIDTH, dtype=np.uint8
            )

        if specular_mask_path:
            self._specular_bits = self._ingest_grayscale_mask(
                specular_mask_path
            )
        if self._specular_bits is None:
            self._specular_bits = np.zeros(
                SPRITE_HEIGHT * SPRITE_WIDTH, dtype=np.uint8
            )

        self._palette = self._extract_palette()

    def compile(self, output_path: Path) -> None:
        """
        Compile the image into a sprite file.

        :param output_path: The path to save the sprite to.
        """

        left, top, right, bottom = calculate_bounding_box(self._source_alpha)

        metadata_bytes = struct.pack(
            SPRITE_METADATA_LAYOUT,
            left,
            top,
            right,
            bottom,
            self._anchor_x,
            self._anchor_y,
            self._encoding.value,
            0,  # palette index placeholder
            self._physics.value,
            0,  # padding
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

        with output_path.open("wb") as f:
            f.write(metadata_bytes + palette_bytes + pixel_bytes)

    # Protected methods

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
        - [7] Specular: Boolean signal (0 or 1).
        - [6] Emission: Boolean signal (0 or 1).
        - [4–5] Alpha: 2-bit transparency (0–3).
        - [0–3] Palette Index: Pointer to one of the 16 colors.

        :returns: The baked pixels.
        """

        indices = self._index_colors() & 0x0F
        alphas = (self._source_alpha.flatten() >> 6) & 0x03
        baked_pixels = (
            self._specular_bits << 7
            | self._emission_bits << 6
            | alphas << 4
            | indices
        )

        return baked_pixels.astype(np.uint8)

    # Protected static methods

    @staticmethod
    def _ingest_grayscale_mask(mask_path: Path) -> EmissionMask | SpecularMask:
        """
        Ingest the grayscale mask.

        :param mask_path: The path to the mask.
        :raises FileNotFoundError: If the mask is missing.
        :raises RuntimeError: If OpenCV fails to read the mask.
        :raises ValueError: If the mask dimensions are not 32×32.
        """

        if not mask_path.exists():
            raise FileNotFoundError(f"Missing mask file: {mask_path}")
        if (make := cv2.imread(mask_path, cv2.IMREAD_GRAYSCALE)) is None:
            raise RuntimeError(f"Could not read mask file: {mask_path}.")

        h, w = make.shape[:2]
        if h != SPRITE_HEIGHT or w != SPRITE_WIDTH:
            raise ValueError(
                f"Mask dimensions must be {SPRITE_WIDTH}×{SPRITE_HEIGHT}."
            )

        return (make.flatten() > 0x80).astype(np.uint8)


def main() -> None:
    parser = ArgumentParser("Sprite Compiler", description=__doc__)
    parser.add_argument(
        "source_image", type=Path, help="Path to the source BGR/RGBA image."
    )
    parser.add_argument(
        "output_path", type=Path, help="Target path for the 1072-byte sprite."
    )
    parser.add_argument(
        "-a",
        "--anchor",
        nargs=2,
        default=(DEFAULT_SPRITE_AX, DEFAULT_SPRITE_AY),
        type=float,
        help=f"Sprite anchor point (x, y). Default is center ({DEFAULT_SPRITE_AX:.2f}, {DEFAULT_SPRITE_AY:.2f}).",
    )
    parser.add_argument(
        "-c",
        "--color_encoding",
        default=ColorEncoding.DEFAULT,
        type=lambda e: ColorEncoding[e.upper()],
        choices=list(ColorEncoding),
        help="Color encoding (Default, Warm, or Cool)",
    )
    parser.add_argument(
        "-p",
        "--physics_type",
        type=lambda p: PhysicsType[p.upper()],
        choices=list(PhysicsType),
        help="Physics type (None, Actor, Static, Sensor, or Projectile)",
        required=True,
    )
    parser.add_argument(
        "-e",
        "--emission_mask",
        default=None,
        type=Path,
        help="Optional grayscale mask for emission mapping.",
    )
    parser.add_argument(
        "-s",
        "--specular_mask",
        default=None,
        type=Path,
        help="Optional grayscale mask for specular mapping.",
    )

    args = parser.parse_args()

    compiler = SpriteCompiler(
        args.source_image, args.anchors, args.color_encoding, args.physics_type
    )
    compiler.ingest_asset(args.emission_mask, args.specular_mask)
    compiler.compile(args.output_path)


if __name__ == "__main__":
    main()
