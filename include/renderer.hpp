/**
 * @file renderer.hpp
 * @brief
 */
#pragma once

#include <Metal/Metal.hpp>

#include "sprite.hpp"

namespace sc {

    /**
     * @class renderer
     * @brief High-level interface for the Metal Compute pipeline.
     *
     * Orchestrates the dispatch of kernels to the GPU using `sc::sprite` data
     * as primary input.
     */
    class renderer final {
    public:
        explicit renderer(MTL::Device* device);
        ~renderer() = default;

        renderer(const renderer&) = delete;
        renderer(renderer&&) = delete;
        renderer& operator=(const renderer&) = delete;
        renderer& operator=(renderer&&) = delete;

        /**
         * @brief Encode a draw command for a single sprite.
         * @param rpd The current render pass descriptor.
         * @param drawable The destination drawable/texture.
         * @param sprite The source data to render.
         * @param pos_x
         * @param pos_y
         */
        void draw(const MTL::RenderPassDescriptor* rpd,
                const MTL::Drawable* drawable, const sprite& sprite,
                float pos_x, float pos_y) const;

    private:
        NS::SharedPtr<MTL::Device> device_;
        NS::SharedPtr<MTL::CommandQueue> command_queue_;
        NS::SharedPtr<MTL::ComputePipelineState> pso_{nullptr};
    };

} // namespace sc
