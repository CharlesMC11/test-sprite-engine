#ifndef SC_GRAPHICS_VISUAL_TYPES_HH
#define SC_GRAPHICS_VISUAL_TYPES_HH

#include "core/core.hh"

namespace sc::graphics {

    using packed_color_t = uint16_t;
    using packed_pixel_t = uint8_t; // [S][E][AA][IIII]

    static SC_CONSTANT unsigned kMaxPaletteSize{16u};

    static SC_CONSTANT packed_pixel_t kMaskPaletteIndex{0x0F};
    static SC_CONSTANT packed_pixel_t kMaskAlpha{0x30};
    static SC_CONSTANT packed_pixel_t kMaskEmission{0x40};
    static SC_CONSTANT packed_pixel_t kMaskSpecular{0x80};

    /**
     * Distribution of color channels across a 16-bit packed integer.
     */
    enum class color_encoding : uint8_t {
        DEFAULT = 0u, // R5G6B5
        WARM, // R6G5B5
        COOL // R5G5B6
    };

    /**
     * A palette of colors packed into 16-bit integers.
     */
    struct alignas(core::kNeonAlignment) palette final {
        packed_color_t colors[kMaxPaletteSize];
    };

} // namespace sc::graphics

#endif // SC_GRAPHICS_VISUAL_TYPES_HH
