#ifndef SC_GRAPHICS_RENDER_CONSTANTS_HH
#define SC_GRAPHICS_RENDER_CONSTANTS_HH

#ifndef __METAL_VERSION__
#include <cstdint>
#endif

namespace sc::render {

    enum metal_buffer_index : uint32_t {
        BUFFER_INDEX_PALETTES,
        BUFFER_INDEX_SPRITES,
        BUFFER_INDEX_X_POSITIONS,
        BUFFER_INDEX_Y_POSITIONS,
        BUFFER_INDEX_Z_POSITIONS,
        BUFFER_INDEX_ATLAS_INDICES,
        BUFFER_INDEX_DRAW_ORDER,
        BUFFER_INDEX_ENTITY_COUNT
    };

} // namespace sc::render

#endif // SC_GRAPHICS_RENDER_CONSTANTS_HH
