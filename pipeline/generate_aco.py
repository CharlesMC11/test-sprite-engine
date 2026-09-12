#!/usr/bin/env -S python3
"""
Generate an Adobe Color Swatch file.
"""

import struct
from argparse import ArgumentParser
from collections.abc import Iterable
from functools import partial

from pipeline import quantize
from pipeline.quantize_color_hex import filter_hex_codes, hex_str

# Public function


def main() -> None:
    parser = ArgumentParser("Adobe Color Swatch Generator", description=__doc__)
    parser.add_argument(
        "-n",
        "--name",
        help="Name of the color palette",
        type=str,
        required=True,
    )
    parser.add_argument(
        "hex_codes",
        help="Hexadecimal color codes to quantize",
        nargs="+",
        type=str,
    )
    args = parser.parse_args()

    generate_aco(args.name, args.hex_codes)


def generate_aco(name: str, hex_codes: Iterable[str]) -> None:

    filtered_codes = filter_hex_codes(hex_codes)
    palette_size = len(filtered_codes)

    v1_header_buf = struct.pack(">HH", 1, palette_size)
    v2_header_buf = struct.pack(">HH", 2, palette_size)

    v1_color_buf = bytearray()
    v2_color_buf = bytearray()
    for code in filtered_codes:
        r, g, b = code

        r16 = _expand_to_16bit(r)
        g16 = _expand_to_16bit(g)
        b16 = _expand_to_16bit(b)

        color_bytes = struct.pack(">HHHHH", 0, r16, g16, b16, 0x0000)
        v1_color_buf.extend(color_bytes)
        v2_color_buf.extend(color_bytes)

        color_name = hex_str(r, g, b)
        name_len_buf = struct.pack(">I", len(color_name) + 1)
        v2_color_buf.extend(name_len_buf)

        name_buf = color_name.encode("utf-16be") + b"\x00\x00"
        v2_color_buf.extend(name_buf)

    combined_buf = bytearray(v1_header_buf)
    combined_buf.extend(v1_color_buf)
    combined_buf.extend(v2_header_buf)
    combined_buf.extend(v2_color_buf)

    with open(f"{name}.aco", "wb") as f:
        f.write(combined_buf)


# Private helpers

_expand_to_16bit = partial(quantize, dst_max=0xFFFF)


if __name__ == "__main__":
    main()
