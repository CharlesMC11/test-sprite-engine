import dataclasses
import enum
import struct
from abc import ABC
from dataclasses import dataclass
from enum import IntEnum, IntFlag
from struct import Struct
from typing import ClassVar, Final, Self

# Core

NEON_ALIGNMENT: Final[int] = 16
CACHE_ALIGNMENT: Final[int] = 128

# Graphics

ASSET_LAYOUT_VERSION: Final[bytes] = b"6"

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


@dataclass(slots=True)
class Metadata(ABC):
    MAGIC: ClassVar[Final[bytes]]
    STRUCT: ClassVar[Final[Struct]]
    EXPECTED_SIZE_BYTES: ClassVar[Final[int]]

    @classmethod
    def from_bytes(cls, buffer: bytes) -> Self:
        """"""

        expected_size = cls.EXPECTED_SIZE_BYTES
        err_msg = f"Buffer size too small for {cls.__name__}. Expected {expected_size}, got"

        buffer_size = len(buffer)
        if buffer_size < expected_size:
            raise ResourceLayoutError(f"{err_msg} {buffer_size}.")

        magic_size = len(cls.MAGIC)
        magic = buffer[:magic_size]
        if magic != cls.MAGIC:
            raise ResourceLayoutError(
                f"Invalid magic bytes! Expected '{cls.MAGIC}' bytes, got {magic}."
            )

        try:
            return cls(*cls.STRUCT.unpack(buffer[magic_size:expected_size]))
        except struct.error:
            raise ResourceLayoutError(f"{err_msg} {len(buffer)}.")

    def to_bytes(self) -> bytes:
        return self.MAGIC + self.__class__.STRUCT.pack(
            *dataclasses.astuple(self)
        )


@dataclass(slots=True)
class SpriteMetadata(Metadata):
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

    MAGIC: ClassVar[Final[bytes]] = b"SC SP v" + ASSET_LAYOUT_VERSION

    STRUCT: ClassVar[Final[Struct]] = Struct("<BBBBffBBBB")
    """The layout of a 24-byte asset metadata.

    - magic
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

    EXPECTED_SIZE_BYTES: ClassVar[Final[int]] = STRUCT.size + len(MAGIC)

    PALETTE_INDEX_OFFSET: ClassVar[Final[int]] = EXPECTED_SIZE_BYTES - 1


SPRITE_MINIMUM_FILE_SIZE_BYTES: Final[int] = (
    SpriteMetadata.EXPECTED_SIZE_BYTES + FOOTER_SIZE_BYTES
)
"""The minimum file size of a sprite file in bytes."""


@dataclass(slots=True)
class AtlasMetadata(Metadata):
    sprite16_count: int
    sprite32_count: int
    palette_count: int

    MAGIC: ClassVar[Final[bytes]] = b"SC AT v" + ASSET_LAYOUT_VERSION

    STRUCT: ClassVar[Final[Struct]] = Struct("<LHH")
    """The layout of an 8-byte atlas metadata.

    - magic
    - sprite16_count (uint32)
    - sprite32_count (uint16)
    - palette_count (uint16)
    """

    EXPECTED_SIZE_BYTES: ClassVar[Final[int]] = STRUCT.size + len(MAGIC)


def is_power_of_2(n: int) -> bool:
    """
    Check if a number is a power of 2.

    :param n: The number to check.
    :return: `True` if it is; `False` otherwise.
    """

    return n > 0 and (n & (n - 1)) == 0


def calculate_padding_needed(n: int, alignment: int) -> int:
    """
    Calculate the needed padding in terms of bytes.

    :param n: The number to be padded.
    :param alignment: The value to be padded up to.

    :return: The number of bytes needed.
    """

    if not is_power_of_2(alignment):
        raise ValueError(f"Alignment must be a power of 2, got {alignment}.")

    return (alignment - n % alignment) % alignment
