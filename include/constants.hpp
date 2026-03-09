/**
 * @file constants.hpp
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

    namespace memory {

        static SC_CONSTANT auto ALIGNMENT{16u};

    } // namespace memory

    namespace paths {

        static SC_CONSTANT auto CHARACTER_ATLAS{"assets/master.atlas"};
        static SC_CONSTANT auto SHADER_LIB{"assets/shader.metallib"};

    } // namespace paths

    namespace ui {

        static SC_CONSTANT uint32_t SCREEN_WIDTH{240u};
        static SC_CONSTANT uint32_t SCREEN_HEIGHT{160u};

    } // namespace ui

} // namespace sc
