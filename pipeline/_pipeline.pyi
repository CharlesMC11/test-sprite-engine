"""Pipeline Bridge Library"""

import enum

MAX_2_BIT: int = 3

MAX_5_BIT: int = 31

MAX_6_BIT: int = 63

NEON_ALIGNMENT: int = 16

CACHE_ALIGNMENT: int = 128

MAX_PALETTE_SIZE: int = 16

PACKED_COLOR_SIZE_BYTES: int = 2

PALETTE_SIZE_BYTES: int = 32

class ColorEncoding(enum.IntEnum):
    """Distribution of color bits across a 2-byte packed integer."""

    NEUTRAL = 0
    """R5G6B5"""

    WARM = 1
    """R6G5B5"""

    COOL = 2
    """R5G5B6"""

class PhysicsType(enum.IntFlag):
    """The laws of physics an entity obeys."""

    __str__ = __repr__

    def __repr__(self, /):
        """Return repr(self)."""

    UNDEFINED = 0

    NONE = 1

    ACTOR = 2

    STATIC = 4

    SENSOR = 8

    PROJECTILE = 16
