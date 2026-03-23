import enum
from enum import IntEnum, IntFlag
from typing import Final

MAX_PALETTE_SIZE: Final[int] = 16
SPRITE_HEIGHT: Final[int] = 32
SPRITE_WIDTH: Final[int] = 32
SPRITE_SIZE_BYTES: Final[int] = 1_072
SPRITE_METADATA: Final[str] = "BBBBBBBB"
"""8-byte sprite metadata. 

- left
- top
- right
- bottom
- anchor_x
- anchor_y
- color_encoding
- phys_type
"""


class ColorEncoding(IntEnum):
    DEFAULT = 1  # R5G6B5
    WARM = enum.auto()  # R6G5B5
    COOL = enum.auto()  # R5G5B6


class PhysicsType(IntFlag):
    NONE = 0x01
    ACTOR = enum.auto()
    STATIC = enum.auto()
    SENSOR = enum.auto()
    PROJECTILE = enum.auto()
