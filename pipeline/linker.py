"""
Asset Linker.

Assembles multiple asset files into a single memory–mappable atlas binary.

The binary layout contains:
- Header (16 bytes): atlas metadata
    - Magic (8 bytes)
    - 16×16 sprite count (4 bytes)
    - 32×32 sprite count (2 bytes)
    - Palette count (2 bytes)
- Data (n bytes): contiguous arrays of palette and sprite structures
    - Color palettes (32 × palette count bytes)
    - 16×16 sprites (272 × 16×16 sprite count bytes)
    - 32×32 sprites (1,040 × 32×32 sprite count bytes)
"""

import re
from argparse import ArgumentParser
from collections.abc import Iterable
from dataclasses import dataclass
from pathlib import Path
from typing import Any

from pipeline import (
    FOOTER_SIZE_BYTES,
    SPRITE_DIMENSIONS_SIZE_BYTES,
    AtlasMetadata,
    ResourceLayoutError,
    SpriteMetadata,
    is_power_of_2,
)

GENERATED_HEADER_FILENAME = "asset_ids.hh"

INDENT = " " * 4
INVALID_IDENTIFIER_SYMBOLS_RE = re.compile(r"\s")


@dataclass(frozen=True, slots=True)
class ComponentBlob:
    blob: bytes
    name: str

    def __eq__(self, other: Any) -> bool:
        return (
            self.blob == other.blob
            if isinstance(other, ComponentBlob)
            else False
        )

    def __hash__(self) -> int:
        return hash(self.blob)


@dataclass(frozen=True, slots=True)
class AssetBlob(ComponentBlob):
    width: int
    height: int


class AssetLinker:
    """Assembles multiple `.asset` files into a singler memory-mappable atlas."""

    # Type annotations

    _metadata: AtlasMetadata

    _palette_registry: dict[ComponentBlob, int]
    _asset_registry: dict[tuple[int, int], tuple[str, list[AssetBlob]]]

    _blobs: list[AssetBlob]

    _current_offset: int
    _sprite8_offset: int
    _sprite16_offset: int
    _sprite32_offset: int
    _sprite64_offset: int

    # Magic methods

    def __init__(self):

        self._palette_registry = {}
        self._asset_registry = {
            (16, 16): ("sprite16", []),
            (32, 32): ("sprite32", []),
        }

        self._sprite8_offset = 0
        self._sprite16_offset = 0
        self._sprite32_offset = 0
        self._sprite64_offset = 0

    def __len__(self) -> int:
        return sum(len(b) for b in self._asset_registry.values())

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

        self.add_asset(paths)

    def add_asset(self, sprite_paths: Iterable[Path]) -> None:
        """
        Ingest and validate sprite binaries.

        :param sprite_paths: Sequence of paths to baked sprite files.

        :raises FileNotFoundError: If a path does not exist.
        :raises ValueError: If the sprite contains invalid magic bytes.
        :raises ResourceLayoutError: If a sprite is less than or equal to 54 bytes in size
            or has invalid dimensions.
        """

        asset_magic_size = len(SpriteMetadata.MAGIC)

        for path in sprite_paths:
            if not path.exists():
                raise FileNotFoundError(f"Sprite not found: {path}.")

            blob = path.read_bytes()
            meta = SpriteMetadata.from_bytes(blob)

            pixels_blob = blob[meta.EXPECTED_SIZE_BYTES : -FOOTER_SIZE_BYTES]
            palette_blob = blob[
                -FOOTER_SIZE_BYTES:-SPRITE_DIMENSIONS_SIZE_BYTES
            ]
            width, height = blob[-SPRITE_DIMENSIONS_SIZE_BYTES:]

            if not (is_power_of_2(width) and is_power_of_2(height)):
                raise ResourceLayoutError(
                    f"'{path.name}' has invalid dimensions: {width}×{height}!"
                )

            name = _clean_identifier(path.stem)

            palette = ComponentBlob(palette_blob, name)
            if palette not in self._palette_registry:
                self._palette_registry[palette] = len(self._palette_registry)

            meta.palette_index = self._palette_registry[palette]

            header_bytes = meta.to_bytes()[asset_magic_size:]
            asset = AssetBlob(header_bytes + pixels_blob, name, width, height)

            if (width, height) in self._asset_registry:
                self._asset_registry[width, height][1].append(asset)
            else:
                raise ResourceLayoutError(
                    f"Unsupported asset dimensions: {width}×{height}."
                )

    def link(self, output_path: Path) -> None:
        """
        Collapse the internal blobs into a single atlas binary.

        The header is 16-byte aligned to ensure the first sprite starts on a
        clean 16-byte boundary.

        :param output_path: The path to save the atlas to.
        """

        header_path = output_path.with_name(GENERATED_HEADER_FILENAME)
        include_guard = f"SC_ASSETS_{header_path.stem.upper()}_HH"
        header_lines = [
            f"#ifndef {include_guard}",
            f"#define {include_guard}\n",
            '#include "core/core.hh"\n',
            "namespace sc::assets {\n",
        ]

        assets_bytes = bytearray()

        blobs, lines = self._generate_enum_contents(
            "palette", self._palette_registry
        )
        assets_bytes.extend(blobs)
        header_lines.extend(lines)

        for name, buffer in self._asset_registry.values():
            blobs, lines = self._generate_enum_contents(name, buffer)
            assets_bytes.extend(blobs)
            header_lines.extend(lines)

        header_lines.extend(
            ("} // namespace sc::assets\n", f"#endif // {include_guard}")
        )

        metadata_bytes = AtlasMetadata(
            len(self._asset_registry[16, 16][1]),
            len(self._asset_registry[32, 32][1]),
            len(self._palette_registry),
        ).to_bytes()

        output_path.write_bytes(metadata_bytes + assets_bytes)
        header_path.write_text("\n".join(header_lines))

    # Protected  methods

    def _generate_enum_contents(
        self, name: str, buffer: Iterable[ComponentBlob]
    ) -> tuple[bytes, list[str]]:
        """
        Generate an enum containing asset IDs.

        :param name: The name to give the enum.
        :param buffer: The buffer to extract the values from.

        :return: The asset blobs and the lines of the enum to write.
        """

        blobs = bytearray()
        lines: list[str] = [self._enum_name(name)]

        for i, asset in enumerate(buffer):
            blobs.extend(asset.blob)
            lines.append(f"{INDENT * 2}{_clean_identifier(asset.name)} = {i}U,")
        lines.extend((f"{INDENT * 2}count", f"{INDENT}}};\n"))

        return blobs, lines

    # Protected static methods

    @staticmethod
    def _enum_name(name: str) -> str:
        return f"{INDENT}enum class {name}_id : core::index_t {{"


# Public functions


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

    linker = AssetLinker()

    if args.file:
        linker.add_from_file_list(args.file)

    if args.input:
        linker.add_asset(args.input)

    if len(linker) == 0:
        raise ValueError("No assets to link.")

    linker.link(args.output_path)


# Protected helpers


def _clean_identifier(identifier: str) -> str:
    """
    Clean a string to be a valid C++ identifier.

    :param identifier: The string to clean.

    :return: The cleaned string.
    """
    return INVALID_IDENTIFIER_SYMBOLS_RE.sub("_", identifier).lower()


if __name__ == "__main__":
    main()
