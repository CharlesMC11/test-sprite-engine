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

- min u, min v, max u, max v (4)
- origin u, origin v (8)
- color encoding (1)
- palette index (1)
- physics type (1)
- padding (1)
"""

ATLAS_METADATA_LAYOUT: Final[str] = "<8sLHH"
"""The layout of a 16-byte atlas metadata.

- magic (8)
- sprite16 count (4)
- sprite32 count (2)
- palette count (2)
"""


class ResourceLayoutError(Exception): ...


class ColorEncoding(IntEnum):
    DEFAULT = 0  # R5G6B5
    WARM = enum.auto()  # R6G5B5
    COOL = enum.auto()  # R5G5B6


class PhysicsType(IntFlag):
    NONE = 1 << 0
    ACTOR = enum.auto()
    STATIC = enum.auto()
    SENSOR = enum.auto()
    PROJECTILE = enum.auto()
