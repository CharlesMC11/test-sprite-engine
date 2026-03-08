import enum
from enum import IntEnum
from typing import Final

SPRITE_HEIGHT: Final[int] = 32
SPRITE_WIDTH: Final[int] = 32
MAX_PALETTE_SIZE: Final[int] = 16

SPRITE_METADATA: Final[str] = "BBBBBBB1x"
"""8-byte sprite metadata. 

- hitbox_min_x
- hitbox_min_y
- hitbox_max_x
- hitbox_max_y
- anchor_x
- anchor_y
- color_encoding
- reserved
"""


class ColorEncoding(IntEnum):
    DEFAULT = 1  # R5G6B5
    WARM = enum.auto()  # R6G5B5
    COOL = enum.auto()  # R5G5B6
