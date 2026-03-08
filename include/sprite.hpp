#pragma once

#include <cstddef>
#include <cstdint>

namespace sc {

    constexpr std::size_t HEIGHT{32};
    constexpr std::size_t WIDTH{32};
    constexpr std::size_t MAX_PALETTE_SIZE{16};

    enum class color_encoding : std::uint8_t {
        DEFAULT = 1, // R5G6B5
        WARM, // R6G5B5
        COOL, // R5G5B6
    };

    using color = std::uint16_t;

    struct pixel {
        std::uint8_t index : 4;
        std::uint8_t alpha : 2;
        std::uint8_t glow : 1;
        std::uint8_t reserved : 1;
    };

    struct alignas(16) sprite {
        std::uint8_t hb_min_x, hb_min_y;
        std::uint8_t hb_max_x, hb_max_y;
        std::uint8_t anchor_x, anchor_y;
        color_encoding encoding;
        std::uint8_t reserved;
        color palette[MAX_PALETTE_SIZE];
        pixel pixels[HEIGHT * WIDTH];
        std::uint64_t padding;
    };

} // namespace sc
