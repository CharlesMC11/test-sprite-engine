import enum
from enum import IntEnum, IntFlag
from typing import Final

MAX_PALETTE_SIZE: Final[int] = 16
SPRITE_HEIGHT: Final[int] = 32
SPRITE_WIDTH: Final[int] = 32
DEFAULT_SPRITE_AX: Final[float] = SPRITE_WIDTH / 2
DEFAULT_SPRITE_AY: Final[float] = SPRITE_HEIGHT / 2
SPRITE_SIZE_BYTES: Final[int] = 1_072
SPRITE_METADATA: Final[str] = "<BBBBffBBBB"
"""16-byte sprite metadata. 

- left, top, right, bottom (4)
- anchor x, anchor y (8)
- color encoding (1)
- palette index (1)
- physics type (1)
- padding (1)
"""


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
