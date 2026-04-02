#ifndef SC_GRAPHICS_VISUAL_TYPES_HH
#define SC_GRAPHICS_VISUAL_TYPES_HH

#ifndef __METAL_VERSION__

#include <ostream>

#endif //__METAL_VERSION__

#include "core/core.hh"

namespace sc::graphics {

    using packed_color_t = uint16_t;
    using packed_pixel_t = uint8_t; // [S][E][AA][IIII]

    static SC_CONSTEXPR uint32_t kMaxPaletteSize{16U};

    static SC_CONSTEXPR uint32_t kMaskPaletteIndex{0x0FU};
    static SC_CONSTEXPR uint32_t kMaskAlpha{0x30U};
    static SC_CONSTEXPR uint32_t kMaskEmission{0x40U};
    static SC_CONSTEXPR uint32_t kMaskSpecular{0x80U};

    /**
     * Distribution of color channels across a 16-bit packed integer.
     */
    enum class color_encoding : uint8_t {
        neutral, // R5G6B5
        warm, // R6G5B5
        cool // R5G5B6
    };

    /**
     * A palette of colors packed into 16-bit integers.
     */
    struct alignas(core::kNeonAlignment) palette final {
        packed_color_t colors[kMaxPaletteSize];
    };

} // namespace sc::graphics

#ifndef __METAL_VERSION__

std::ostream& operator<<(std::ostream&, const sc::graphics::palette&);

#endif // __METAL_VERSION__

#endif // SC_GRAPHICS_VISUAL_TYPES_HH
