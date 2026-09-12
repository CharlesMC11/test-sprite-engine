#!/usr/bin/env -S python3
"""
Quantize a list of hexadecimal color codes to 6‑bit and 5‑bit representations,
then display the entered and quantized colors in a tabular format along with
their ANSI color representations.
"""

from argparse import ArgumentParser
from collections.abc import Iterable
from functools import partial

from pipeline import (
    expand_from_5bit,
    expand_from_6bit,
    quantize_to_5bit,
    quantize_to_6bit,
)

# Public function


def main() -> None:
    parser = ArgumentParser(name=__name__, description=__doc__)
    parser.add_argument(
        "hex_codes",
        help="Hexadecimal color codes to quantize",
        nargs="+",
        type=str,
    )
    args = parser.parse_args()

    quantize_color_hex_codes(args.hex_codes)


def quantize_color_hex_codes(hex_codes: Iterable[str]) -> None:

    unique_codes: set[tuple[int, int, int]] = filter_hex_codes(hex_codes)

    palette_size = len(unique_codes)
    bit_count = max(palette_size - 1, 0).bit_length()
    unit = "bit" if bit_count == 1 else "bits"

    print(
        f"{palette_size} colors ({bit_count} {unit})\n"
        f"{'Entered':<7} {'Neutral':^14} {'Warm':^14} {'Cool':^14} {'Patches':<7}"
    )

    for r, g, b in unique_codes:
        # 6-bit
        r6_quantized = quantize_to_6bit(r)
        g6_quantized = quantize_to_6bit(g)
        b6_quantized = quantize_to_6bit(b)

        r6_expanded = expand_from_6bit(r6_quantized)
        g6_expanded = expand_from_6bit(g6_quantized)
        b6_expanded = expand_from_6bit(b6_quantized)

        # 5-bit
        r5_quantized = quantize_to_5bit(r)
        g5_quantized = quantize_to_5bit(g)
        b5_quantized = quantize_to_5bit(b)

        r5_expanded = expand_from_5bit(r5_quantized)
        g5_expanded = expand_from_5bit(g5_quantized)
        b5_expanded = expand_from_5bit(b5_quantized)

        _print_codes(
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


def filter_hex_codes(
    raw_hex_codes: Iterable[str],
) -> list[tuple[int, int, int]]:
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

    return sorted(result)


def hex_str(r: int, g: int, b: int) -> str:
    return f"#{r:02X}{g:02X}{b:02X}"


# Protected helpers


def _print_codes(
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
        f"{_ansi_str(r, g, b)}"
        f"{_ansi_str(r5_expanded, g6_expanded, b5_expanded)}"
        f"{_ansi_str(r6_expanded, g5_expanded, b5_expanded)}"
        f"{_ansi_str(r5_expanded, g5_expanded, b6_expanded)}"
    )

    print(
        entered,
        f"{neutral_expanded} [{neutral_quantized:04X}]",
        f"{warm_expanded} [{warm_quantized:04X}]",
        f"{cool_expanded} [{cool_quantized:04X}]",
        patches,
    )


def _ansi_str(r: int, g: int, b: int) -> str:
    return f"\x1b[48;2;{r};{g};{b}m  \x1b[0m"


if __name__ == "__main__":
    main()
