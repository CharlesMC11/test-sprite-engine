#pragma once

#include <Metal/Metal.hpp>

#include "sprite.hpp"

namespace sc {

    class renderer {
    public:
        explicit renderer(MTL::Device* device);

        void draw(const MTL::RenderPassDescriptor* rpd,
                const MTL::Drawable* drawable, const sprite& sprite) const;

    private:
        NS::SharedPtr<MTL::Device> device_;
        NS::SharedPtr<MTL::CommandQueue> command_queue_;
        NS::SharedPtr<MTL::ComputePipelineState> pso_{nullptr};
    };

} // namespace sc
