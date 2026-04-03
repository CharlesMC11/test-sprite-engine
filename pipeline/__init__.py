import enum
from enum import IntEnum, IntFlag
from typing import Final

# Core

NEON_ALIGNMENT: Final[int] = 16
CACHE_ALIGNMENT: Final[int] = 128

# Graphics

MAX_PALETTE_SIZE: Final[int] = 16
"""The max number of unique colors in an asset."""

SPRITE_METADATA_LAYOUT: Final[str] = "<BBBBffBBBB"
"""The layout of a 16-byte sprite metadata.

- bbox
    - u_min (uint8), u_max (uint8)
    - v_min (uint8), v_max (uint8)
- anchor
    - u_anchor (float32)
    - v_anchor (float32)
- depth (uint8)
- physics_type (uint8)
- color_encoding (uint8)
- palette_index (uint8)
"""

SPRITE_METADATA_SIZE_BYTES: Final[int] = 16
"""The total size of the sprite metadata in bytes."""

PALETTE_INDEX_OFFSET: Final[int] = SPRITE_METADATA_SIZE_BYTES - 1
"""Byte offset of `palette_index` in the sprite metadata."""

PACKED_COLOR_SIZE_BYTES: Final[int] = 2
"""The size of a packed color in bytes."""

PALETTE_SIZE_BYTES: Final[int] = MAX_PALETTE_SIZE * 2
"""The total size of all 2-byte colors in bytes."""

SPRITE_DIMENSIONS_SIZE_BYTES: Final[int] = 2
"""The total size of the sprite dimensions in bytes."""

SPRITE_FOOTER_SIZE_BYTES: Final[int] = (
    PALETTE_SIZE_BYTES + SPRITE_DIMENSIONS_SIZE_BYTES
)
""" The total size of the sprite footer in bytes."""

SPRITE_MINIMUM_FILE_SIZE_BYTES: Final[int] = (
    SPRITE_METADATA_SIZE_BYTES + SPRITE_FOOTER_SIZE_BYTES
)
"""The minimum file size of a sprite file in bytes."""

ATLAS_METADATA_LAYOUT: Final[str] = "<8sLHH"
"""The layout of a 16-byte atlas metadata.

- magic (uint64)
- sprite16_count (uint32)
- sprite32_count (uint16)
- palette_count (uint16)
"""


class ResourceLayoutError(Exception): ...


class ResourceLayoutWarning(UserWarning): ...


class ColorEncoding(IntEnum):
    """Distribution of color bits across a 2-byte packed integer."""

    NEUTRAL = 0  # R5G6B5
    WARM = enum.auto()  # R6G5B5
    COOL = enum.auto()  # R5G5B6


class PhysicsType(IntFlag):
    UNDEFINED = 0
    NONE = enum.auto()
    ACTOR = enum.auto()
    STATIC = enum.auto()
    SENSOR = enum.auto()
    PROJECTILE = enum.auto()
