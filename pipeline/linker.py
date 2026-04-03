"""
Atlas Linker.

Assembles multiple sprite files into a single memory–mappable atlas binary.

The binary layout contains:
- Header (16 bytes): atlas metadata
    - Magic (8 bytes): 'SC AT v4'
    - 16×16 sprite count (4 bytes)
    - 32×32 sprite count (2 bytes)
    - Palette count (2 bytes)
- Data (n bytes): contiguous arrays of palette and sprite structures
    - Color palettes (32 × palette count bytes)
    - 16×16 sprites (272 × 16×16 sprite count bytes)
    - 32×32 sprites (1,040 × 32×32 sprite count bytes)
"""

import struct
from argparse import ArgumentParser
from collections.abc import Sequence
from pathlib import Path
from typing import Final

from pipeline import (
    ATLAS_METADATA_LAYOUT,
    PALETTE_INDEX_OFFSET,
    SPRITE_DIMENSIONS_SIZE_BYTES,
    SPRITE_MINIMUM_FILE_SIZE_BYTES,
    SPRITE_PALETTE_SIZE_BYTES,
    ResourceLayoutError,
)

ATLAS_MAGIC: Final[bytes] = b"SC AT v4"


class AtlasLinker:
    """Assembles multiple `.sprite` files into a singler memory-mappable atlas."""

    # Type annotations

    _palette_blobs: list[bytes]
    _palette_names: list[str]

    _sprite8_blobs: list[bytearray]
    _sprite8_names: list[str]

    _sprite16_blobs: list[bytearray]
    _sprite16_names: list[str]

    _sprite32_blobs: list[bytearray]
    _sprite32_names: list[str]

    _sprite64_blobs: list[bytearray]
    _sprite64_names: list[str]

    # Magic methods

    def __init__(self):
        self._palette_blobs = []
        self._palette_names = []

        self._sprite8_blobs = []
        self._sprite8_names = []

        self._sprite16_blobs = []
        self._sprite16_names = []

        self._sprite32_blobs = []
        self._sprite32_names = []

        self._sprite64_blobs = []
        self._sprite64_names = []

    def __len__(self) -> int:
        return (
            len(self._sprite8_blobs)
            + len(self._sprite16_blobs)
            + len(self._sprite32_blobs)
            + len(self._sprite64_blobs)
        )

    # Public methods

    def add_from_file_list(self, list_path: Path) -> None:
        """
        Add sprites to the linker from a text file.

        :param list_path: The path to the text file container file paths.
        """

        paths: list[Path] = []

        with list_path.open("r") as f:
            for line in f:
                if path := line.strip():
                    paths.append(Path(path).expanduser())

        self.add_sprites(paths)

    def add_sprites(self, sprite_paths: Sequence[Path]) -> None:
        """
        Ingest and validate sprite binaries.

        :param sprite_paths: Sequence of paths to baked sprite files.
        :raises FileNotFoundError: If a path does not exist.
        :raises ResourceLayoutError: If a sprite is less than or equal to 54 bytes in size
            or has invalid dimensions.
        """

        pixels_end = SPRITE_PALETTE_SIZE_BYTES + SPRITE_DIMENSIONS_SIZE_BYTES
        for path in sprite_paths:
            if not path.exists():
                raise FileNotFoundError(f"Sprite not found: {path}.")

            blob = path.read_bytes()
            if len(blob) <= SPRITE_MINIMUM_FILE_SIZE_BYTES:
                raise ResourceLayoutError(
                    f"'{path.name}' must be bigger than {SPRITE_MINIMUM_FILE_SIZE_BYTES} bytes."
                )

            sprite_blob = bytearray(blob[:-pixels_end])
            palette_blob = blob[-pixels_end:-SPRITE_DIMENSIONS_SIZE_BYTES]
            height, width = blob[-SPRITE_DIMENSIONS_SIZE_BYTES:]

            if palette_blob not in self._palette_blobs:
                self._palette_blobs.append(palette_blob)
                self._palette_names.append(f"{path.stem}_palette")

            sprite_blob[PALETTE_INDEX_OFFSET] = self._palette_blobs.index(
                palette_blob
            )

            if height == 8 and width == 8:
                blob_dst = self._sprite8_blobs
                name_dst = self._sprite8_names
            elif height == 16 and width == 16:
                blob_dst = self._sprite16_blobs
                name_dst = self._sprite16_names
            elif height == 32 and width == 32:
                blob_dst = self._sprite32_blobs
                name_dst = self._sprite32_names
            elif height == 64 and width == 64:
                blob_dst = self._sprite64_blobs
                name_dst = self._sprite64_names
            else:
                raise ResourceLayoutError(
                    f"'{path.name}' has invalid dimensions: {width}{height}!"
                )

            blob_dst.append(sprite_blob)
            name_dst.append(path.stem)

    def link(self, output_path: Path) -> None:
        """
        Collapse the internal blobs into a single atlas binary.

        The header is 16-byte aligned to ensure the first sprite starts on a
        clean 16-byte boundary.

        :param output_path: The path to save the atlas to.
        """

        metadata_bytes = struct.pack(
            ATLAS_METADATA_LAYOUT,
            ATLAS_MAGIC,
            len(self._sprite16_blobs),
            len(self._sprite32_blobs),
            len(self._palette_blobs),
        )

        with output_path.open("wb") as f:
            f.write(metadata_bytes)
            for palette in self._palette_blobs:
                f.write(palette)
            for sprite in self._sprite16_blobs:
                f.write(sprite)
            for sprite in self._sprite32_blobs:
                f.write(sprite)

        if self._palette_blobs:
            self._generate_enum_header(
                output_path.with_name(f"palette_index.hh"),
                self._palette_names,
            )

        if self._sprite16_blobs:
            self._generate_enum_header(
                output_path.with_name(f"sprite16_index.hh"),
                self._sprite16_names,
            )

        if self._sprite32_blobs:
            self._generate_enum_header(
                output_path.with_name(f"sprite32_index.hh"),
                self._sprite32_names,
            )

    # Protected static methods

    @staticmethod
    def _generate_enum_header(
        header_path: Path, enumerators: Sequence[str]
    ) -> None:
        """
        Generate a C++ enum that maps a sprite to its atlas index.

        :param header_path: The path to save the header to.
        :param enumerators: The names of the enumerators.
        """

        indent = " " * 4

        lines = [
            "#pragma once",
            "",
            '#include "core/core.hh"',
            "",
            "namespace sc::assets {",
            "",
            f"{indent}enum class {header_path.stem} : core::index_t {{",
        ]

        for name in enumerators:
            clean_name = name.replace(" ", "_").replace("-", "_").upper()
            lines.append(f"{indent * 2}{clean_name},")

        lines.extend(
            (
                f"{indent}}};",
                "",
                "} // namespace sc::assets",
                "",
            )
        )

        header_path.write_text("\n".join(lines))


def main() -> None:
    parser = ArgumentParser("Atlas Linker", description=__doc__)
    parser.add_argument(
        "output_path", type=Path, help="Target path for the sprite atlas."
    )
    parser.add_argument(
        "-i", "--input", nargs="*", type=Path, help="Paths to the sprite files."
    )
    parser.add_argument(
        "-f",
        "--file",
        type=Path,
        help="Text file containing paths to sprite files (1 per line).",
    )

    args = parser.parse_args()

    linker = AtlasLinker()

    if args.file:
        linker.add_from_file_list(args.file)

    if args.input:
        linker.add_sprites(args.input)

    if len(linker) == 0:
        raise ValueError("No sprites to link.")

    linker.link(args.output_path)


if __name__ == "__main__":
    main()
