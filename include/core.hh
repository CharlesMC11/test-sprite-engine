/**
 * @file core.hh
 * @brief
 */
#pragma once

#ifdef __METAL_VERSION__
#define SC_CONSTANT constant constexpr
#else
#define SC_CONSTANT constexpr
#include <cstdint>
#endif

namespace sc {

    namespace sys {

        using index_t = uint32_t;
        using atlas_index_t = index_t;

        static SC_CONSTANT auto kAlignment{16u};

    } // namespace sys

    namespace assets {

        static SC_CONSTANT auto kCharacterAtlas{"assets/master.atlas"};
        static SC_CONSTANT auto kShaderLib{"assets/shader.metallib"};

    } // namespace assets

    namespace display {

        static SC_CONSTANT uint32_t kWidth{240u};
        static SC_CONSTANT uint32_t kHeight{160u};

    } // namespace display

} // namespace sc
