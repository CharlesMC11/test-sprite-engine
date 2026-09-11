#!/usr/bin/env -S python3
"""
Quantize a list of hexadecimal color codes to 6‑bit and 5‑bit representations,
then display the entered and quantized colors in a tabular format along with
their ANSI color representations.
"""

from argparse import ArgumentParser
from collections.abc import Iterable


def main(hex_codes: Iterable[str]) -> None:

    unique_codes: set[tuple[int, int, int]] = filter_hex_codes(hex_codes)

    palette_size = len(unique_codes)
    bit_count = max(palette_size - 1, 0).bit_length()
    unit = "bit" if bit_count == 1 else "bits"

    print(
        f"{palette_size} colors ({bit_count}\u202f{unit})\n"
        f"{'Entered':<7} {'Neutral':^14} {'Warm':^14} {'Cool':^14} {'Patches':<7}"
    )

    for r, g, b in sorted(unique_codes):
        # 6-bit
        r6_quantized, r6_expanded = quantize_and_expand(r, 6)
        g6_quantized, g6_expanded = quantize_and_expand(g, 6)
        b6_quantized, b6_expanded = quantize_and_expand(b, 6)

        # 5-bit
        r5_quantized, r5_expanded = quantize_and_expand(r, 5)
        g5_quantized, g5_expanded = quantize_and_expand(g, 5)
        b5_quantized, b5_expanded = quantize_and_expand(b, 5)

        print_codes(
            r,
            g,
            b,
            r6_quantized,
            g6_quantized,
            b6_quantized,
            r5_quantized,
            g5_quantized,
            b5_quantized,
            r6_expanded,
            g6_expanded,
            b6_expanded,
            r5_expanded,
            g5_expanded,
            b5_expanded,
        )


def filter_hex_codes(raw_hex_codes: Iterable[str]) -> set[tuple[int, int, int]]:
    result: set[tuple[int, int, int]] = set()

    for raw_code in raw_hex_codes:
        cleaned_code = raw_code.strip().removeprefix("#")

        if len(cleaned_code) != 6:
            continue
        try:
            r, g, b = bytes.fromhex(cleaned_code)
        except ValueError:
            continue

        result.add((r, g, b))

    return result


def quantize_and_expand(value: int, bit_count: int) -> tuple[int, int]:

    max_val = (0x01 << bit_count) - 0x01
    quantized = (value * max_val + 0x7F) // 0xFF

    return quantized, (quantized * 0xFF + max_val // 2) // max_val


def print_codes(
    r: int,
    g: int,
    b: int,
    r6_quantized: int,
    g6_quantized: int,
    b6_quantized: int,
    r5_quantized: int,
    g5_quantized: int,
    b5_quantized: int,
    r6_expanded: int,
    g6_expanded: int,
    b6_expanded: int,
    r5_expanded: int,
    g5_expanded: int,
    b5_expanded: int,
) -> None:
    entered = hex_str(r, g, b)

    neutral_quantized = (
        (r5_quantized << 11) | (g6_quantized << 5) | b5_quantized
    )
    warm_quantized = (r6_quantized << 10) | (g5_quantized << 5) | b5_quantized
    cool_quantized = (r5_quantized << 11) | (g5_quantized << 5) | b6_quantized

    neutral_expanded = hex_str(r5_expanded, g6_expanded, b5_expanded)
    warm_expanded = hex_str(r6_expanded, g5_expanded, b5_expanded)
    cool_expanded = hex_str(r5_expanded, g5_expanded, b6_expanded)

    patches = (
        f"{ansi_str(r, g, b)}"
        f"{ansi_str(r5_expanded, g6_expanded, b5_expanded)}"
        f"{ansi_str(r6_expanded, g5_expanded, b5_expanded)}"
        f"{ansi_str(r5_expanded, g5_expanded, b6_expanded)}"
    )

    print(
        entered,
        f"{neutral_expanded} [{neutral_quantized:04X}]",
        f"{warm_expanded} [{warm_quantized:04X}]",
        f"{cool_expanded} [{cool_quantized:04X}]",
        patches,
    )


def hex_str(r: int, g: int, b: int) -> str:
    return f"#{r:02X}{g:02X}{b:02X}"


def ansi_str(r: int, g: int, b: int) -> str:
    return f"\x1b[48;2;{r};{g};{b}m  \x1b[0m"


if __name__ == "__main__":
    parser = ArgumentParser()
    parser.add_argument(
        "hex_codes",
        help="Hexadecimal color codes to quantize",
        nargs="+",
        type=str,
    )
    args = parser.parse_args()

    main(args.hex_codes)
