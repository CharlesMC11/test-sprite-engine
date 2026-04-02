#ifndef SC_GRAPHICS_DISPLAY_CONSTANTS_HH
#define SC_GRAPHICS_DISPLAY_CONSTANTS_HH

#ifndef __METAL_VERSION__
#include <cstdint>
#endif

#include "core/core.hh"

namespace sc::display {

    static SC_CONSTEXPR uint32_t kWidth{240U};
    static SC_CONSTEXPR uint32_t kHeight{160U};

    static SC_CONSTEXPR float kDefaultR{0.50f};
    static SC_CONSTEXPR float kDefaultG{0.50f};
    static SC_CONSTEXPR float kDefaultB{0.50f};

    static SC_CONSTEXPR float kTargetFPS{60.0f};

} // namespace sc::display

#endif // SC_GRAPHICS_DISPLAY_CONSTANTS_HH
