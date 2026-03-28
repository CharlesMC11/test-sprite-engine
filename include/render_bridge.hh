/**
 * @file render_bridge.hh
 * @brief
 */
#pragma once

#include <Metal/Metal.hpp>

#include "atlas.hh"
#include "file_mapping.hh"
#include "scene_registry.hh"

namespace sc {

    /**
     * @class render_bridge
     * @brief High-level interface for the Metal Compute pipeline.
     *
     * Orchestrates the dispatch of kernels to the GPU using `sc::sprite` data
     * as primary input.
     */
    class render_bridge final {
    public:
        explicit render_bridge(MTL::Device* device);
        render_bridge(const render_bridge&) = delete;
        render_bridge(render_bridge&&) = delete;

        ~render_bridge() = default;

        render_bridge& operator=(const render_bridge&) = delete;
        render_bridge& operator=(render_bridge&&) = delete;

        void set_sprite_atlas(const core::file_mapping<sprites::atlas>& atlas);

        /**
         * @brief
         * @param buffer The destination drawable/texture.
         */
        void begin_frame(const MTL::Drawable* buffer);

        /**
         * @brief
         */
        void end_frame(const MTL::Drawable* buffer);

        void clear() const;

        /**
         * @brief Encode a draw command for entities on screen.
         * @param registry
         */
        void draw(const scene_registry& registry) const;

    private:
        NS::SharedPtr<MTL::Device> device_;
        NS::SharedPtr<MTL::CommandQueue> queue_;
        NS::SharedPtr<MTL::ComputePipelineState> clear_pso_{nullptr};
        NS::SharedPtr<MTL::ComputePipelineState> sprite_pso_{nullptr};
        NS::SharedPtr<MTL::Buffer> sprite_buffer_{nullptr};
        NS::UInteger palette_offset_{0u};
        NS::UInteger sprites_offset_{0u};

        MTL::CommandBuffer* command_buffer_{nullptr};
        MTL::ComputeCommandEncoder* encoder_{nullptr};
    };

} // namespace sc
