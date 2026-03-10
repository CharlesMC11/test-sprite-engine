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

    namespace sys {

        static SC_CONSTANT auto ALIGNMENT{16u};
        using ENTITY_ID_T = uint32_t;

    } // namespace sys

    namespace assets {

        static SC_CONSTANT auto CHARACTER_SPRITE_BANK{"assets/master.atlas"};
        static SC_CONSTANT auto SHADER_LIB{"assets/shader.metallib"};

    } // namespace assets

    namespace display {

        static SC_CONSTANT uint32_t WIDTH{240u};
        static SC_CONSTANT uint32_t HEIGHT{160u};

    } // namespace display

} // namespace sc
