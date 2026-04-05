"""
Asset Compiler.

Bakes a source (BGR/RGBA) image and optional emission & specular masks
into a specialized binary format of at least 58 bytes.

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

import struct
from argparse import ArgumentParser
from collections.abc import Sequence
from dataclasses import dataclass
from pathlib import Path
from warnings import warn

import cv2
import numpy as np
import numpy.typing as npt

from pipeline import (
    MAX_PALETTE_SIZE,
    NEON_ALIGNMENT,
    SPRITE_MINIMUM_FILE_SIZE_BYTES,
    ColorEncoding,
    PhysicsType,
    ResourceLayoutError,
    ResourceLayoutWarning,
    SpriteMetadata,
    calculate_padding_needed,
    is_power_of_2,
)

# Types

type BGRImage = npt.NDArray[np.uint8]
type AlphaMask = npt.NDArray[np.uint8]
type EmissionMask = npt.NDArray[np.uint8]
type SpecularMask = npt.NDArray[np.uint8]
type Palette = npt.NDArray[np.uint8]

type PackedColors = npt.NDArray[np.uint16]
type BakedPixels = npt.NDArray[np.uint8]


@dataclass(frozen=True, slots=True)
class SpriteComponents:
    metadata: SpriteMetadata
    bgr_image: BGRImage
    alpha_mask: AlphaMask | None
    emission_mask: EmissionMask | None
    specular_mask: SpecularMask | None
    width: int
    height: int


# Public functions


def main() -> None:
    parser = ArgumentParser("Asset Compiler", description=__doc__)
    parser.add_argument(
        "source_image", type=Path, help="Path to the source BGR/RGBA image."
    )
    parser.add_argument(
        "output_path", type=Path, help="Target path for the asset binary."
    )
    parser.add_argument(
        "-c",
        "--color_encoding",
        default=ColorEncoding.NEUTRAL,
        type=lambda e: ColorEncoding[e.upper()],
        choices=list(ColorEncoding),
        help="Color encoding (Neutral, Warm, or Cool). Default is Neutral.",
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
        "-d",
        "--depth",
        default=0,
        type=int,
        help='"Thickness" of the sprite. Default is 0.',
    )
    parser.add_argument(
        "-p",
        "--physics_type",
        default=PhysicsType.UNDEFINED,
        type=lambda p: PhysicsType[p.upper()],
        choices=list(PhysicsType),
        help="Physics type (None, Actor, Static, Sensor, or Projectile).",
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

    asset_components = ingest_asset(
        args.source_image,
        args.color_encoding,
        args.anchor,
        args.depth,
        args.physics_type,
        args.emission_mask,
        args.specular_mask,
    )
    compile_asset(args.output_path, asset_components)


def ingest_asset(
    bgra_image_path: Path,
    color_encoding: ColorEncoding,
    anchor: Sequence[float],
    depth: int,
    physics_type: PhysicsType,
    emission_mask_path: Path | None,
    specular_mask_path: Path | None,
) -> SpriteComponents:
    """
    Validate the source asset.

    Split the source image into color and alpha channels, flatten the pixel
    grid for evaluation, and pre-threshold the glow mask.

    :param bgra_image_path: The path to the BGRA image.
    :param color_encoding: The target encoding.
    :param anchor: The anchor points.
    :param depth: The asset’s 'thickness'.
    :param physics_type: The laws of physics this asset obeys.
    :param emission_mask_path: Optional path to a grayscale emission mask.
    :param specular_mask_path: Optional path to a grayscale specular mask.

    :raises FileNotFoundError: If the assets can’t be found.
    :raises RuntimeError: If OpenCV fails to read the assets.
    :raises ResourceLayoutError: If the image dimensions are not a power of 2.
    """

    # Validate source image

    if not bgra_image_path.exists():
        raise FileNotFoundError(f"Missing image file: {bgra_image_path}")

    if (image := cv2.imread(bgra_image_path, cv2.IMREAD_UNCHANGED)) is None:
        raise RuntimeError("Could not read source image.")

    height, width = image.shape[:2]
    if not (is_power_of_2(height) and is_power_of_2(width)):
        raise ResourceLayoutError(
            f"Source image '{bgra_image_path.name}' dimensions must be a "
            f"power of 2 (8, 16, 32, etc...)! Got {width}×{height}."
        )

    # Extract alpha mask

    if image.shape[2] >= 4:
        source_bgr = image[:, :, :3]
        source_alpha = image[:, :, 3]
    else:
        source_bgr = image
        source_alpha = None

    # Calculate bounding box

    if source_alpha is not None:
        u_min, u_max, v_min, v_max = _calculate_bounding_box(source_alpha)
    else:
        u_min = v_min = 0
        u_max = width - 1
        v_max = height - 1

    # Validate anchor points

    u_anchor, v_anchor = anchor

    if np.isnan(u_anchor):
        u_anchor = width / 2.0
    elif u_anchor < u_min or u_anchor > u_max:
        raise ValueError(
            "U anchor point out of range! "
            f"Expected to be in [0, {width}), got {u_anchor}."
        )

    if np.isnan(v_anchor):
        v_anchor = height / 2.0
    elif v_anchor < v_min or v_anchor > v_max:
        raise ValueError(
            "V anchor point out of range! "
            f"Expected to be in [0, {height}), got {v_anchor}."
        )

    # Validate depth

    if depth < 0x00 or depth > 0xFF:
        raise ValueError(
            "Depth out of or range! "
            f"Expected to be in [{0x00}, {0xFF}], got {depth}."
        )

    # Extract emission mask

    if emission_mask_path is not None:
        source_emission = _ingest_grayscale_mask(
            emission_mask_path, width, height
        )
    else:
        source_emission = None

    # Extract specular mask

    if specular_mask_path is not None:
        source_specular = _ingest_grayscale_mask(
            specular_mask_path, width, height
        )
    else:
        source_specular = None

    # Construct components

    header = SpriteMetadata(
        u_min,
        u_max,
        v_min,
        v_max,
        u_anchor,
        v_anchor,
        depth,
        physics_type,
        color_encoding,
    )
    return SpriteComponents(
        header,
        source_bgr,
        source_alpha,
        source_emission,
        source_specular,
        width,
        height,
    )


def compile_asset(output_path: Path, components: SpriteComponents) -> None:
    """
    Bake the asset into a specialized binary.

    :param output_path: The path to save the binary to.
    :param components: The asset pixel_components to bake.

    :raises ResourceLayoutError: If the baked asset does not follow the expected format.
    """

    meta = components.metadata
    header_bytes = meta.to_bytes()

    pixels, palette = _bake_pixels(components, output_path.name)
    pixel_bytes = pixels.tobytes(order="C")
    padding_needed = calculate_padding_needed(len(pixel_bytes), NEON_ALIGNMENT)
    pixel_bytes += b"\x00" * padding_needed

    palette_bytes = bytearray()
    palette_size = len(palette)
    packed_palette = _pack_colors_to_16bit(palette, meta.color_encoding)
    for i in range(MAX_PALETTE_SIZE):
        if i < palette_size:
            color = packed_palette[i]
            palette_bytes.extend(struct.pack("<H", color))
        else:
            palette_bytes.extend(struct.pack("<H", 0x0000))

    footer_bytes = struct.pack("<BB", components.width, components.height)

    combined_buffer = header_bytes + pixel_bytes + palette_bytes + footer_bytes
    sprite_size_bytes = (
        SPRITE_MINIMUM_FILE_SIZE_BYTES
        + components.height * components.width
        + padding_needed
    )
    if len(combined_buffer) != sprite_size_bytes:
        raise ResourceLayoutError(
            f"Buffer size mismatch for {output_path.name}! "
            f"Expected {sprite_size_bytes} bytes, got {len(combined_buffer)} bytes "
            f"(Metadata: {len(header_bytes)} bytes, Pixels: {len(pixel_bytes)} bytes, "
            f"Palette: {len(palette_bytes)} bytes, Dimensions: {len(footer_bytes)})."
        )

    output_path.write_bytes(combined_buffer)


# Protected helpers


def _calculate_bounding_box(mask: AlphaMask) -> tuple[int, int, int, int]:
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


def _ingest_grayscale_mask(
    mask_path: Path, source_image_width: int, source_image_height: int
) -> EmissionMask | SpecularMask:
    """
    Ingest the grayscale mask.

    :param mask_path: The path to the mask.
    :param source_image_width: The source image width.
    :param source_image_height: The source image height.

    :raises FileNotFoundError: If the mask file does not exist.
    :raises RuntimeError: If OpenCV fails to read the mask.
    :raises ResourceLayoutError: If the mask dimensions do not match source image.
    """

    if not mask_path.exists():
        raise FileNotFoundError(f"Missing mask file: '{mask_path}'")

    if (mask := cv2.imread(mask_path, cv2.IMREAD_GRAYSCALE)) is None:
        raise RuntimeError(f"Could not read mask file: '{mask_path}'.")

    height, width = mask.shape[:2]
    if width != source_image_width or height != source_image_height:
        raise ResourceLayoutError(
            f"Mask '{mask_path.name}' dimensions do not match source image! "
            f"Expected {source_image_width}×{source_image_height}, got {width}×{height}."
        )

    return mask


def _bake_pixels(
    pixel_components: SpriteComponents, asset_name: str
) -> tuple[BakedPixels, Palette]:
    """
    Bake the pixel components into a final 8-bit format.

    The Bit Mapping:
    - [7] Specular: Boolean signal (0 or 1).
    - [6] Emission: Boolean signal (0 or 1).
    - [4–5] Alpha: 2-bit transparency (0–3).
    - [0–3] Palette Index: Pointer to one of the 16 colors.

    :param pixel_components: The components to bake.
    :param asset_name: The asset identifier.

    :returns: The baked pixels.
    """

    bgr_flat_array = pixel_components.bgr_image.reshape(-1, 3)
    palette = _extract_palette(bgr_flat_array, asset_name)
    index = _index_colors(bgr_flat_array, palette) & 0x0F

    width = pixel_components.width
    height = pixel_components.height

    if pixel_components.alpha_mask is not None:
        flattened_alpha = pixel_components.alpha_mask.flatten(order="C")
        alpha = (flattened_alpha >> 6) & 0x03
    else:
        alpha = np.full(height * width, 0x03, dtype=np.uint8)

    if pixel_components.emission_mask is not None:
        flattened_emission = pixel_components.emission_mask.flatten(order="C")
        emission = (flattened_emission > 0x08) & 0x01
    else:
        emission = np.zeros(height * width, dtype=np.uint8)

    if pixel_components.specular_mask is not None:
        flattened_specular = pixel_components.specular_mask.flatten(order="C")
        specular = (flattened_specular > 0x08) & 0x01
    else:
        specular = np.zeros(height * width, dtype=np.uint8)

    baked_pixels = specular << 7 | emission << 6 | alpha << 4 | index
    return baked_pixels.astype(np.uint8), palette


def _extract_palette(bgr_array: BGRImage, asset_name: str) -> Palette:
    """
    Extract 16 unique colors from the source image.

    :param bgr_array: The flattened bgr pixels array from the source image.
    :param asset_name: The asset identifier.

    :returns: The 16 unique colors that were extracted.
    """

    unique_colors, per_color_pixel_count = np.unique(
        bgr_array, axis=0, return_counts=True
    )

    color_count = len(unique_colors)
    if color_count > MAX_PALETTE_SIZE:
        warn(
            f"'{asset_name}' has {color_count} unique colors. "
            f"It will be truncated to {MAX_PALETTE_SIZE}.",
            ResourceLayoutWarning,
        )

    top_indices = np.argsort(-per_color_pixel_count)[:MAX_PALETTE_SIZE]
    top_colors = unique_colors[top_indices]

    sorted_indices = np.lexsort(
        (top_colors[:, 0], top_colors[:, 1], top_colors[:, 2])
    )

    return top_colors[sorted_indices]


def _index_colors(bgr_array: BGRImage, palette: Palette):
    """
    Calculate the color indices for the extracted palette.

    :param bgr_array: The flattened bgr pixels array from the source image.
    :param palette: The top 16 unique colors from the source image.

    :returns: The indices to colors in the palette.
    """

    color_deltas = bgr_array[:, np.newaxis, :] - palette
    distances = np.linalg.norm(color_deltas, axis=2)

    return np.argmin(distances, axis=1)


def _pack_colors_to_16bit(
    bgr_matrix: BGRImage, encoding: ColorEncoding
) -> PackedColors:
    """
    Pack the 8-bit BGR channels into 2-byte integers.

    :param bgr_matrix: The original color channels to pack.
    :param encoding: The color encoding mode to use.

    :returns: The packed 2-byte colors.
    """

    b, g, r = bgr_matrix.astype(np.uint16).T

    if encoding == ColorEncoding.NEUTRAL:
        packed_colors = (r >> 3) << 11 | (g >> 2) << 5 | b >> 3

    elif encoding == ColorEncoding.WARM:
        packed_colors = (r >> 2) << 10 | (g >> 3) << 5 | b >> 3

    elif encoding == ColorEncoding.COOL:
        packed_colors = (r >> 3) << 11 | (g >> 3) << 5 | b >> 2

    else:
        raise ValueError("Invalid color mode.")

    return packed_colors


if __name__ == "__main__":
    main()
