#ifndef SC_GRAPHICS_DISPLAY_CONSTANTS_HH
#define SC_GRAPHICS_DISPLAY_CONSTANTS_HH

#include "core/core.hh"

namespace sc::display {

    static SC_CONSTANT unsigned kWidth{240u};
    static SC_CONSTANT unsigned kHeight{160u};

    static SC_CONSTANT float kDefaultR{0.50f};
    static SC_CONSTANT float kDefaultG{0.50f};
    static SC_CONSTANT float kDefaultB{0.50f};

    static SC_CONSTANT unsigned kTargetFPS{60u};

} // namespace sc::display

#endif // SC_GRAPHICS_DISPLAY_CONSTANTS_HH
