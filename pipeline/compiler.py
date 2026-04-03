"""
Sprite Compiler.

Bakes a source (BGR/RGBA) image and optional emission & specular masks
into a specialized binary format of at least 54 bytes.

The binary layout contains:
- Header (16 bytes): sprite metadata
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
- Pixels (height×width bytes): 8-bit packed values [S][E][AA][IIII]
- Footer (34 bytes):
    - Color palette (32 bytes): 16 2-byte unique colors
    - Height (1 byte): height in pixels
    - Width (1 byte): width in pixels
"""

import struct
from argparse import ArgumentParser
from collections.abc import Sequence
from pathlib import Path
from warnings import warn

import cv2
import numpy as np
import numpy.typing as npt

from pipeline import (
    MAX_PALETTE_SIZE,
    SPRITE_METADATA_LAYOUT,
    SPRITE_MINIMUM_FILE_SIZE_BYTES,
    ColorEncoding,
    PhysicsType,
    ResourceLayoutError,
)

type BGRImage = npt.NDArray[np.uint8]
type AlphaMask = npt.NDArray[np.uint8]
type EmissionMask = npt.NDArray[np.uint8]
type SpecularMask = npt.NDArray[np.uint8]
type Palette = npt.NDArray[np.uint8]

type PackedColors = npt.NDArray[np.uint16]
type BakedPixels = npt.NDArray[np.uint8]


class ResourceLayoutWarning(UserWarning): ...


class SpriteCompiler:
    """Compiles a source image into a sprite file."""

    # Type annotations

    _source_path: Path
    _color_encoding: ColorEncoding
    _physics_type: PhysicsType
    _u_anchor: float
    _v_anchor: float
    _width: int
    _height: int
    _depth: int
    _u_min: int
    _u_max: int
    _v_min: int
    _v_max: int
    _source_bgr: BGRImage | None
    _source_alpha: AlphaMask | None
    _emission_bits: EmissionMask | None
    _specular_bits: SpecularMask | None
    _palette: Palette | None
    _pixel_flat_array: BGRImage | None

    # Magic methods

    def __init__(
        self,
        source_path: Path,
        color_encoding: ColorEncoding,
        anchor: Sequence[float],
        depth: int,
        physics_type: PhysicsType,
    ):
        self._source_path = source_path

        self._color_encoding = color_encoding
        self._u_anchor, self._v_anchor = anchor
        self._depth = depth
        self._physics_type = physics_type

        self._width = self._height = 0
        self._u_min = self._u_max = self._v_min = self._v_max = -1

        self._source_bgr = None
        self._source_alpha = None
        self._emission_bits = None
        self._specular_bits = None

        self._palette = None
        self._pixel_flat_array = None

    # Properties

    @property
    def pixels_size_bytes(self) -> int:
        return self._height * self._width

    @property
    def sprite_size_bytes(self) -> int:
        return SPRITE_MINIMUM_FILE_SIZE_BYTES + self.pixels_size_bytes

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

        # Validate dimensions

        height, width = image.shape[:2]
        if not (is_power_of_2(height) and is_power_of_2(width)):
            raise ResourceLayoutError(
                f"Source image '{self._source_path.name}' dimensions must be a "
                f"power of 2 (8, 16, 32, etc...)! Got {width}×{height}."
            )

        self._width = width
        self._height = height

        # Extract alpha bits

        if image.shape[2] >= 4:
            self._source_bgr = image[:, :, :3]
            self._source_alpha = image[:, :, 3]
        else:
            self._source_bgr = image
            self._source_alpha = np.full((height, width), 0xFF, dtype=np.uint8)

        # Get bounding box

        self._u_min, self._u_max, self._v_min, self._v_max = (
            calculate_bounding_box(self._source_alpha)
        )

        # Validate anchor points

        if np.isnan(self._u_anchor):
            self._u_anchor = self._width / 2.0
        elif self._u_anchor < self._u_min or self._u_anchor > self._u_max:
            raise ValueError(
                "U anchor point out of range! "
                f"Expected to be in [0, {self._width}), got {self._u_anchor}."
            )

        if np.isnan(self._v_anchor):
            self._v_anchor = self._height / 2.0
        elif self._v_anchor < self._v_min or self._v_anchor > self._v_max:
            raise ValueError(
                "V anchor point out of range! "
                f"Expected to be in [0, {self._height}), got {self._v_anchor}."
            )

        # Validate depth

        if self._depth < 0x00 or self._depth > 0xFF:
            raise ValueError(
                "Depth out of or range! "
                f"Expected to be in [{0x00}, {0xFF}], got {self._depth}."
            )

        # Extract emission bits

        if emission_mask_path:
            self._emission_bits = self._ingest_grayscale_mask(
                emission_mask_path
            )
        if self._emission_bits is None:
            self._emission_bits = np.zeros(height * width, dtype=np.uint8)

        # Extract specular bits

        if specular_mask_path:
            self._specular_bits = self._ingest_grayscale_mask(
                specular_mask_path
            )
        if self._specular_bits is None:
            self._specular_bits = np.zeros(height * width, dtype=np.uint8)

        # Construct sprite elements

        self._pixel_flat_array = self._source_bgr.reshape(-1, 3)
        self._palette = self._extract_palette()

    def compile(self, output_path: Path) -> None:
        """
        Compile the image into a sprite file.

        :param output_path: The path to save the sprite to.
        """

        metadata_bytes = struct.pack(
            SPRITE_METADATA_LAYOUT,
            self._u_min,
            self._u_max,
            self._v_min,
            self._v_max,
            self._u_anchor,
            self._v_anchor,
            self._depth,
            self._physics_type.value,
            self._color_encoding.value,
            0x3F,  # ASCII for '?'
        )

        pixel_bytes = self._bake_pixels().tobytes()

        palette_bytes = bytearray()
        palette_size = len(self._palette)
        packed_palette = pack_colors_to_16bit(
            self._palette, self._color_encoding
        )
        for i in range(MAX_PALETTE_SIZE):
            if i < palette_size:
                color = packed_palette[i]
                palette_bytes.extend(struct.pack("<H", color))
            else:
                palette_bytes.extend(struct.pack("<H", 0x3F))

        footer_bytes = struct.pack(
            "<BB",
            self._width,
            self._height,
        )

        combined_buffer = (
            metadata_bytes + pixel_bytes + palette_bytes + footer_bytes
        )
        if len(combined_buffer) != self.sprite_size_bytes:
            raise ResourceLayoutError(
                f"Buffer size mismatch for {output_path.name}! "
                f"Expected {self.sprite_size_bytes} bytes, got {combined_buffer} bytes "
                f"(Metadata: {len(metadata_bytes)} bytes, Pixels: {len(pixel_bytes)} bytes, "
                f"Palette: {len(palette_bytes)} bytes, Dimensions: {len(footer_bytes)})."
            )

        output_path.write_bytes(combined_buffer)

    # Protected methods

    def _ingest_grayscale_mask(
        self, mask_path: Path
    ) -> EmissionMask | SpecularMask:
        """
        Ingest the grayscale mask.

        :param mask_path: The path to the mask.
        :raises FileNotFoundError: If the mask is missing.
        :raises RuntimeError: If OpenCV fails to read the mask.
        :raises ValueError: If the mask dimensions are not 32×32.
        """

        if not mask_path.exists():
            raise FileNotFoundError(f"Missing mask file: '{mask_path}'")

        if (mask := cv2.imread(mask_path, cv2.IMREAD_GRAYSCALE)) is None:
            raise RuntimeError(f"Could not read mask file: '{mask_path}'.")

        height, width = mask.shape[:2]
        if width != self._width or height != self._height:
            raise ResourceLayoutError(
                f"Mask '{mask_path.name}' dimensions do not match source image! "
                f"Expected {self._width}×{self._height}, got {width}×{height}."
            )

        return (mask.flatten(order="C") > 0x80).astype(np.uint8)

    def _extract_palette(self) -> Palette:
        """
        Extract 16 unique colors from the source image.

        :returns: The 16 unique colors that were extracted.
        """

        unique_colors, per_color_pixel_count = np.unique(
            self._pixel_flat_array, axis=0, return_counts=True
        )

        color_count = len(unique_colors)
        if color_count > MAX_PALETTE_SIZE:
            warn(
                f"'{self._source_path.name}' has {color_count} unique colors. "
                f"It will be truncated to {MAX_PALETTE_SIZE}.",
                ResourceLayoutWarning,
            )

        top_indices = np.argsort(-per_color_pixel_count)[:MAX_PALETTE_SIZE]
        top_colors = unique_colors[top_indices]

        sorted_indices = np.lexsort(
            (top_colors[:, 0], top_colors[:, 1], top_colors[:, 2])
        )

        return top_colors[sorted_indices]

    def _index_colors(self):
        """
        Calculate the color indices for the extracted palette.

        :returns: The indices to colors in the palette.
        """

        color_deltas = self._pixel_flat_array[:, np.newaxis, :] - self._palette
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
        alphas = (self._source_alpha.flatten(order="C") >> 6) & 0x03
        baked_pixels = (
            self._specular_bits << 7
            | self._emission_bits << 6
            | alphas << 4
            | indices
        )

        return baked_pixels.astype(np.uint8)

    # Protected static methods


def is_power_of_2(n: int) -> bool:
    """
    Check if a number is a power of 2.

    :param n: The number to check.
    :return: `True` if it is; `False` otherwise.
    """

    return n > 0 and (n & (n - 1)) == 0


def calculate_bounding_box(mask: AlphaMask) -> tuple[int, int, int, int]:
    """
    Calculate the bounding box of a sprite based from an alpha mask.

    :param mask: An alpha mask to calculate the bounding box from.
    :returns: The top-left and bottom-right coordinates of the bounding box.
    """

    visible_coords = np.argwhere(mask > 0)
    if visible_coords.size > 0:
        v_min, u_min = visible_coords.min(axis=0)
        v_max, u_max = visible_coords.max(axis=0)
    else:
        u_min = u_max = v_min = v_max = 0

    return u_min, u_max, v_min, v_max


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

    if encoding == ColorEncoding.NEUTRAL:
        packed_colors = (r >> 3) << 11 | (g >> 2) << 5 | b >> 3

    elif encoding == ColorEncoding.WARM:
        packed_colors = (r >> 2) << 10 | (g >> 3) << 5 | b >> 3

    elif encoding == ColorEncoding.COOL:
        packed_colors = (r >> 3) << 11 | (g >> 3) << 5 | b >> 2

    else:
        raise ValueError("Invalid color mode.")

    return packed_colors


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
        default=(float("NaN"), float("Nan")),
        type=float,
        help=f"Sprite anchor (u, v). Default is center.",
    )
    parser.add_argument(
        "-d", "--depth", default=0, type=int, help='"Thickness" of the sprite.'
    )
    parser.add_argument(
        "-c",
        "--color_encoding",
        default=ColorEncoding.NEUTRAL,
        type=lambda e: ColorEncoding[e.upper()],
        choices=list(ColorEncoding),
        help="Color encoding (Default, Warm, or Cool)",
    )
    parser.add_argument(
        "-p",
        "--physics_type",
        default=PhysicsType.UNDEFINED,
        type=lambda p: PhysicsType[p.upper()],
        choices=list(PhysicsType),
        help="Physics type (None, Actor, Static, Sensor, or Projectile)",
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
        args.source_image,
        args.color_encoding,
        args.anchor,
        args.depth,
        args.physics_type,
    )
    compiler.ingest_asset(args.emission_mask, args.specular_mask)
    compiler.compile(args.output_path)


if __name__ == "__main__":
    main()
