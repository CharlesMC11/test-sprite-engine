/**
 * @file render_bridge.hpp
 * @brief
 */
#pragma once

#include <Metal/Metal.hpp>

#include "entity_layout.hpp"
#include "sprite.hpp"
#include "sprite_bank.hpp"

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
        ~render_bridge() = default;

        render_bridge(const render_bridge&) = delete;
        render_bridge(render_bridge&&) = delete;
        render_bridge& operator=(const render_bridge&) = delete;
        render_bridge& operator=(render_bridge&&) = delete;

        void set_sprite_bank(const sprite_bank& bank);

        /**
         * @brief
         * @param buffer The destination drawable/texture.
         */
        void begin_frame(const MTL::Drawable* buffer);

        void clear(const MTL::Drawable* buffer);

        /**
         * @brief Encode a draw command for a single sprite.
         * @param sprite The source data to render.
         * @param pos_x
         * @param pos_y
         */
        void draw(const sprite& sprite, float pos_x, float pos_y) const;

        void draw(const entity_layout& layout) const;

        /**
         * @brief
         */
        void end_frame(const MTL::Drawable* buffer);

    private:
        NS::SharedPtr<MTL::Device> device_;
        NS::SharedPtr<MTL::CommandQueue> queue_;
        NS::SharedPtr<MTL::ComputePipelineState> clear_pso_{nullptr};
        NS::SharedPtr<MTL::ComputePipelineState> sprite_pso_{nullptr};
        NS::SharedPtr<MTL::Buffer> sprite_buffer_{nullptr};

        MTL::CommandBuffer* command_buffer_{nullptr};
        MTL::ComputeCommandEncoder* encoder_{nullptr};
    };

} // namespace sc
