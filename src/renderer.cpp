#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#define NS_PRIVATE_IMPLEMENTATION

#include "renderer.hpp"

#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

#include <iostream>

#include "constants.hpp"

namespace sc {

    renderer::renderer(MTL::Device* device)
        : device_{NS::TransferPtr(device)},
          command_queue_{NS::TransferPtr(device_->newCommandQueue())}
    {
        NS::Error* error{nullptr};

        const auto* library_path{
                NS::String::string(paths::SHADER_LIB, NS::UTF8StringEncoding)};
        auto* library{device->newLibrary(library_path, &error)};

        const auto* fn_name{
                NS::String::string("render_sprite", NS::UTF8StringEncoding)};
        auto* function{library->newFunction(fn_name)};

        pso_ = NS::TransferPtr(
                device_->newComputePipelineState(function, &error));
        if (!pso_) [[unlikely]]
            std::cerr << error->localizedDescription() << std::endl;

        function->release();
        library->release();
    }

    void renderer::begin_frame(const MTL::Drawable* drawable)
    {
        buffer_ = command_queue_->commandBuffer();
        encoder_ = buffer_->computeCommandEncoder();
        encoder_->setComputePipelineState(pso_.get());

        const auto* out_texture{
                reinterpret_cast<const CA::MetalDrawable*>(drawable)
                        ->texture()};
        encoder_->setTexture(out_texture, 0);
    }

    void renderer::draw(
            const sprite& sprite, const float pos_x, const float pos_y) const
    {
        encoder_->setBytes(&sprite, sizeof(sprite), 0);

        const struct {
            float x;
            float y;
        } pos{pos_x, pos_y};
        encoder_->setBytes(&pos, sizeof(pos), 1);

        const auto grid_size{MTL::Size(SPRITE_WIDTH, SPRITE_HEIGHT, 1)};
        const auto thread_group_size{MTL::Size(8, 8, 1)};
        encoder_->dispatchThreads(grid_size, thread_group_size);
    }

    void renderer::end_frame(const MTL::Drawable* drawable)
    {
        encoder_->endEncoding();
        buffer_->presentDrawable(drawable);
        buffer_->commit();

        encoder_ = nullptr;
        buffer_ = nullptr;
    }

} // namespace sc
