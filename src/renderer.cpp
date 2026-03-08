#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#define NS_PRIVATE_IMPLEMENTATION

#include "renderer.hpp"

#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

namespace sc {

    renderer::renderer(MTL::Device* device)
        : device_{NS::TransferPtr(device)},
          command_queue_{NS::TransferPtr(device_->newCommandQueue())}
    {
        NS::Error* error{nullptr};

        const auto* library_path{NS::String::string(
                "build/shader.metallib", NS::UTF8StringEncoding)};
        auto* library{device->newLibrary(library_path, &error)};

        const auto* fn_name{
                NS::String::string("render_sprite", NS::UTF8StringEncoding)};
        auto* function{library->newFunction(fn_name)};

        pso_ = NS::TransferPtr(
                device_->newComputePipelineState(function, &error));

        function->release();
        library->release();
    }

    void renderer::draw(const MTL::RenderPassDescriptor* rpd,
            const MTL::Drawable* drawable, const sprite& sprite) const
    {
        if (!(rpd && drawable))
            return;

        auto* buffer{command_queue_->commandBuffer()};
        auto* encoder{buffer->computeCommandEncoder()};

        encoder->setComputePipelineState(pso_.get());

        encoder->setBytes(&sprite, sizeof(sprite), 0);

        const auto* out_texture{
                reinterpret_cast<const CA::MetalDrawable*>(drawable)
                        ->texture()};
        encoder->setTexture(out_texture, 0);

        const auto grid_size{MTL::Size(SC_SPRITE_WIDTH, SC_SPRITE_HEIGHT, 1)};
        const auto thread_group_size{MTL::Size(8, 8, 1)};
        encoder->dispatchThreads(grid_size, thread_group_size);

        encoder->endEncoding();
        buffer->presentDrawable(drawable);
        buffer->commit();
    }

} // namespace sc
