import enum
from enum import IntEnum, IntFlag
from typing import Final

MAX_PALETTE_SIZE: Final[int] = 16
"""The max number of unique colors in a sprite."""

SPRITE_HEIGHT: Final[int] = 32
"""The sprite height in pixels."""

SPRITE_WIDTH: Final[int] = 32
"""The sprite width in pixels."""

SPRITE_METADATA_SIZE_BYTES: Final[int] = 16
"""The sprite metadata size in bytes."""

SPRITE_PIXELS_SIZE_BYTES: Final[int] = SPRITE_HEIGHT * SPRITE_WIDTH
"""The total size of all 1-byte color indices in bytes."""

SPRITE_PALETTE_SIZE_BYTES: Final[int] = MAX_PALETTE_SIZE * 2
"""The total size of all 2-byte colors in bytes."""

SPRITE_SIZE_BYTES: Final[int] = (
    SPRITE_METADATA_SIZE_BYTES
    + SPRITE_PALETTE_SIZE_BYTES
    + SPRITE_PIXELS_SIZE_BYTES
)
"""The total size of a sprite file in bytes."""

SPRITE_METADATA_LAYOUT: Final[str] = "<BBBBffBBBB"
"""The layout of a 16-byte sprite metadata. 

- u min, v min, u max, v max (4)
- pivot x, pivot y (8)
- color encoding (1)
- palette index (1)
- physics type (1)
- padding (1)
"""

ATLAS_METADATA_LAYOUT: Final[str] = "<8sLL"
"""The layout of a 16-byte atlas metadata.

- magic (8)
- palette count (4)
- sprite count (4)
"""


class ResourceLayoutError(Exception): ...


class ColorEncoding(IntEnum):
    DEFAULT = 1  # R5G6B5
    WARM = enum.auto()  # R6G5B5
    COOL = enum.auto()  # R5G5B6


class PhysicsType(IntFlag):
    NONE = 1 << 0
    ACTOR = enum.auto()
    STATIC = enum.auto()
    SENSOR = enum.auto()
    PROJECTILE = enum.auto()
