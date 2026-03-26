"""
Atlas Linker.

Assembles multiple 1,072-byte sprite files into a single memory–mappable
atlas binary.

The binary layout contains:
- Header (16 bytes):
    - Magic (8 bytes): "SC AT v3" (spaces added).
    - Palette count (4 bytes): uint32 sprite count.
    - Sprite count (4 bytes): uint32 sprite count.
- Data (1,072 bytes × n): Contiguous array of sprite structures.
"""

import struct
from argparse import ArgumentParser
from collections.abc import Sequence
from pathlib import Path
from typing import Final

from pipeline import (
    SPRITE_PALETTE_SIZE_BYTES,
    ATLAS_METADATA_LAYOUT,
    ResourceLayoutError,
    SPRITE_SIZE_BYTES,
)

ATLAS_MAGIC: Final[bytes] = b"SC AT v3"
PALETTE_INDEX_ENUM_NAME: Final[str] = "palette_index"
SPRITE_INDEX_ENUM_NAME: Final[str] = "sprite_index"


class AtlasLinker:
    """Assembles multiple `.sprite` files into a singler memory-mappable atlas."""

    # Type annotations

    _palette_blobs: list[bytes]
    _palette_names: list[str]
    _sprite_blobs: list[bytearray]
    _sprite_names: list[str]

    # Magic methods

    def __init__(self):
        self._palette_blobs = []
        self._palette_names = []
        self._sprite_blobs = []
        self._sprite_names = []

    def __len__(self) -> int:
        return len(self._sprite_blobs)

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
        :raises ValueError: If a sprite is not 1,072-bytes in size.
        """

        for path in sprite_paths:
            if not path.exists():
                raise FileNotFoundError(f"Sprite not found: {path}.")

            blob = path.read_bytes()
            if len(blob) != SPRITE_SIZE_BYTES:
                raise ResourceLayoutError(
                    f"{path.name} must be {SPRITE_SIZE_BYTES} bytes."
                )

            sprite_blob = bytearray(blob[:-SPRITE_PALETTE_SIZE_BYTES])
            palette_blob = blob[-SPRITE_PALETTE_SIZE_BYTES:]

            if palette_blob not in self._palette_blobs:
                self._palette_blobs.append(palette_blob)
                self._palette_names.append(f"{path.stem}_palette")

            sprite_blob[13] = self._palette_blobs.index(palette_blob)
            self._sprite_blobs.append(sprite_blob)
            self._sprite_names.append(path.stem)

    def link(self, output_path: Path) -> None:
        """
        Collapse the internal blobs into a single atlas binary.

        The header is 16-byte aligned to ensure the first sprite starts on a
        clean 16-byte boundary.

        :param output_path: The path to save the atlas to.
        """

        palette_count = len(self._palette_blobs)
        sprite_count = len(self._sprite_blobs)
        header = struct.pack(
            ATLAS_METADATA_LAYOUT, ATLAS_MAGIC, palette_count, sprite_count
        )

        with output_path.open("wb") as f:
            f.write(header)
            for palette in self._palette_blobs:
                f.write(palette)
            for sprite in self._sprite_blobs:
                f.write(sprite)

        self._generate_enum_header(
            output_path.with_name(f"{PALETTE_INDEX_ENUM_NAME}.hh"),
            self._palette_names,
        )

        self._generate_enum_header(
            output_path.with_name(f"{SPRITE_INDEX_ENUM_NAME}.hh"),
            self._sprite_names,
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
            '#include "core.hh"',
            "",
            "namespace sc::sprites {",
            "",
            f"{indent}enum class {header_path.stem} : core::atlas_index_t {{",
        ]

        for name in enumerators:
            clean_name = name.replace(" ", "_").replace("-", "_").upper()
            lines.append(f"{indent * 2}{clean_name},")

        lines.extend(
            (
                f"{indent}}};",
                "",
                "} // namespace sc::sprites",
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
