"""
Atlas Linker.

Assembles multiple 1,072-byte sprite files into a single memory–mappable
atlas binary.

The binary layout contains:
- Header (16 bytes):
    - Magic (8 bytes): b"SC ATLAS" (space added).
    - Count (8 bytes): uint64 sprite count.
- Data (N * 1,072 bytes): Contiguous array of sprite structures.
"""

import struct
from argparse import ArgumentParser
from collections.abc import Sequence
from pathlib import Path
from typing import Final

from pipeline import SPRITE_SIZE_BYTES

SPRITE_BANK_MAGIC: Final[bytes] = b"SC ATLAS"
ENUM_NAME: Final[str] = "atlas_index"


class AtlasLinker:
    """Assembles multiple `.sprite` files into a singler memory-mappable atlas."""

    # Type annotations

    _sprite_blobs: list[bytes]
    _sprite_names: list[str]

    # Magic methods

    def __init__(self):
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
                raise ValueError(
                    f"{path.name} must be {SPRITE_SIZE_BYTES} bytes."
                )

            self._sprite_blobs.append(blob)
            self._sprite_names.append(path.stem)

    def link(self, output_path: Path) -> None:
        """
        Collapse the internal blobs into a single atlas binary.

        The header is 16-byte aligned to ensure the first sprite starts on a
        clean 16-byte boundary.

        :param output_path: The path to save the atlas to.
        """

        count = len(self._sprite_blobs)
        header = struct.pack("<8sQ", SPRITE_BANK_MAGIC, count)

        with output_path.open("wb") as f:
            f.write(header)
            for blob in self._sprite_blobs:
                f.write(blob)

        self._generate_header(output_path.with_name(f"{ENUM_NAME}.hh"))

    # Protected methods

    def _generate_header(self, header_path: Path) -> None:
        """
        Generate a C header mapping sprite names to their atlas indices.

        :param header_path: The path to save the header to.
        """

        lines = [
            "#pragma once",
            "",
            '#include "core.hh"',
            "",
            "namespace sc::sprites {",
            "",
            f"{' ' * 4}enum class {ENUM_NAME} : core::atlas_index_t {{",
        ]

        for i, name in enumerate(self._sprite_names):
            clean_name = name.replace(" ", "_").replace("-", "_").upper()
            lines.append(f"{' ' * 8}{clean_name},")

        lines.extend(
            [
                f"{' ' * 4}}};",
                "",
                "} // namespace sc::sprites",
                "",
            ]
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
