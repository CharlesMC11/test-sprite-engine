import enum
from enum import IntEnum, IntFlag
from typing import Final

MAX_PALETTE_SIZE: Final[int] = 16
"""The max number of unique colors in a sprite."""

SPRITE_METADATA_SIZE_BYTES: Final[int] = 16
"""The sprite metadata size in bytes."""

SPRITE_PALETTE_SIZE_BYTES: Final[int] = MAX_PALETTE_SIZE * 2
"""The total size of all 2-byte colors in bytes."""

SPRITE_DIMENSIONS_SIZE_BYTES: Final[int] = 2
"""The total size of the sprite dimensions in bytes."""

SPRITE_MINIMUM_FILE_SIZE_BYTES: Final[int] = (
    SPRITE_METADATA_SIZE_BYTES
    + SPRITE_PALETTE_SIZE_BYTES
    + SPRITE_DIMENSIONS_SIZE_BYTES
)
"""The minimum file size of a sprite file in bytes."""

SPRITE_METADATA_LAYOUT: Final[str] = "<BBBBffBBBB"
"""The layout of a 16-byte sprite metadata. 

- bbox (4 bytes)
    - u_min, u_max (2 bytes)
    - v_min, v_max (2 bytes)
- anchor (8 bytes)
    - u_anchor (4 bytes)
    - v_anchor (4 bytes)
- depth (1 byte)
- physics_type (1 byte)
- color_encoding (1 byte)
- palette_index (1 byte)
"""

PALETTE_INDEX_OFFSET: Final[int] = 15
"""Byte offset of `palette_index` in the sprite metadata."""

ATLAS_METADATA_LAYOUT: Final[str] = "<8sLHH"
"""The layout of a 16-byte atlas metadata.

- magic (8 bytes)
- sprite16_count (4 bytes)
- sprite32_count (2 bytes)
- palette_count (2 bytes)
"""


class ResourceLayoutError(Exception): ...


class ColorEncoding(IntEnum):
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
