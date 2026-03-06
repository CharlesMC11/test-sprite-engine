from enum import IntEnum
import enum

HEIGHT = 32
WIDTH = 32
MAX_PALETTE_SIZE = 16

SPRITE_METADATA = "BBBBBBB1x"
"""min_x, min_y, max_x, max_y, anchor_x, anchor_y, color_encoding, reserved"""


class ColorEncoding(IntEnum):
    DEFAULT = 1  # R5G6B5
    WARM = enum.auto()  # R6G5B5
    COOL = enum.auto()  # R5G5B6
