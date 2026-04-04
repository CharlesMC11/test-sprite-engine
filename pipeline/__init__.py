import enum
import struct
from dataclasses import dataclass
from enum import IntEnum, IntFlag
from typing import ClassVar, Final, Self
from warnings import warn

# Core

NEON_ALIGNMENT: Final[int] = 16
CACHE_ALIGNMENT: Final[int] = 128

# Graphics

MAX_PALETTE_SIZE: Final[int] = 16
"""The max number of unique colors in an asset."""

PACKED_COLOR_SIZE_BYTES: Final[int] = 2
"""The size of a packed color in bytes."""

PALETTE_SIZE_BYTES: Final[int] = PACKED_COLOR_SIZE_BYTES * MAX_PALETTE_SIZE
"""The total size of all 2-byte colors in bytes."""

SPRITE_DIMENSIONS_SIZE_BYTES: Final[int] = 2
"""The total size of the sprite dimensions in bytes."""

FOOTER_SIZE_BYTES: Final[int] = (
    PALETTE_SIZE_BYTES + SPRITE_DIMENSIONS_SIZE_BYTES
)
""" The total size of the sprite footer in bytes."""

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
    """The laws of physics an entity obeys."""

    UNDEFINED = 0
    NONE = enum.auto()
    ACTOR = enum.auto()
    STATIC = enum.auto()
    SENSOR = enum.auto()
    PROJECTILE = enum.auto()


@dataclass(frozen=True, slots=True)
class SpriteMetadata:
    u_min: int
    u_max: int
    v_min: int
    v_max: int
    u_anchor: float
    v_anchor: float
    depth: int
    physics_type: PhysicsType
    color_encoding: ColorEncoding
    palette_index: int = 0x3F

    LAYOUT: ClassVar[Final[str]] = "<BBBBffBBBB"
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

    EXPECTED_SIZE_BYTES: ClassVar[Final[int]] = 16

    PALETTE_INDEX_OFFSET: ClassVar[Final[int]] = EXPECTED_SIZE_BYTES - 1

    @classmethod
    def from_bytes(cls, buffer: bytes) -> Self:
        n = cls.EXPECTED_SIZE_BYTES
        if len(buffer) > n:
            warn(
                f"Buffer is larger than {n}! Using only the first {n} bytes.",
                ResourceLayoutWarning,
            )

        return cls(*struct.unpack(cls.LAYOUT, buffer[:n]))

    def to_bytes(self) -> bytes:
        return struct.pack(
            self.LAYOUT,
            self.u_min,
            self.u_max,
            self.v_min,
            self.v_max,
            self.u_anchor,
            self.v_anchor,
            self.depth,
            self.physics_type,
            self.color_encoding,
            self.palette_index,
        )


SPRITE_MINIMUM_FILE_SIZE_BYTES: Final[int] = (
    SpriteMetadata.EXPECTED_SIZE_BYTES + FOOTER_SIZE_BYTES
)
"""The minimum file size of a sprite file in bytes."""
