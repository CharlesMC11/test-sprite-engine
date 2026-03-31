#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#define NS_PRIVATE_IMPLEMENTATION

#include "graphics/metal_bridge.hh"

#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

#include <iostream>

#include "assets/asset_constants.hh"
#include "assets/atlas.hh"
#include "core/mapped_view.hh"
#include "graphics/display_constants.hh"
#include "graphics/graphics_types.hh"
#include "render_constants.hh"

namespace sc::render {

    // Constructors

    metal_bridge::metal_bridge(MTL::Device* device)
        : device_{NS::TransferPtr(device)},
          queue_{NS::TransferPtr(device_->newCommandQueue())}
    {
        NS::Error* error{nullptr};

        const auto* library_path{
                NS::String::string(assets::kShaderLib, NS::UTF8StringEncoding)};
        auto* library{device->newLibrary(library_path, &error)};

        const auto* fn_name{
                NS::String::string("k_clear_screen", NS::UTF8StringEncoding)};
        auto* function{library->newFunction(fn_name)};

        clear_pso_ = NS::TransferPtr(
                device_->newComputePipelineState(function, &error));
        if (!clear_pso_) [[unlikely]] {
            std::cerr << error->localizedDescription() << '\n';
            throw;
        }

        fn_name = NS::String::string("k_draw_sprites", NS::UTF8StringEncoding);
        function = library->newFunction(fn_name);

        sprite_pso_ = NS::TransferPtr(
                device_->newComputePipelineState(function, &error));
        if (!sprite_pso_) [[unlikely]] {
            std::cerr << error->localizedDescription() << '\n';
            throw;
        }

        function->release();
        library->release();
    }

    // Public methods

    void metal_bridge::begin_frame(const MTL::Drawable* buffer)
    {
        command_buffer_ = queue_->commandBuffer();
        encoder_ = command_buffer_->computeCommandEncoder();

        const auto* out_texture{
                reinterpret_cast<const CA::MetalDrawable*>(buffer)->texture()};
        encoder_->setTexture(out_texture, 0UZ);
    }

    void metal_bridge::end_frame(const MTL::Drawable* buffer)
    {
        encoder_->endEncoding();
        command_buffer_->presentDrawable(buffer);
        command_buffer_->commit();

        encoder_ = nullptr;
        command_buffer_ = nullptr;
    }

    void metal_bridge::clear() const
    {
        encoder_->setComputePipelineState(clear_pso_.get());

        const auto grid_size = MTL::Size(display::kWidth, display::kHeight, 1U);
        const auto thread_group_size = MTL::Size(16U, 16U, 1U);

        encoder_->dispatchThreads(grid_size, thread_group_size);
    }

    void metal_bridge::draw(const entity_registry& registry) const
    {
        encoder_->setComputePipelineState(sprite_pso_.get());

        encoder_->setBuffer(sprite32_buffer_.get(), palette_span_offset_,
                BUFFER_INDEX_PALETTES);
        encoder_->setBuffer(sprite32_buffer_.get(), sprite32_span_offset_,
                BUFFER_INDEX_SPRITES);

        using reg = entity_registry;

        encoder_->setBuffer(registry.xform_buffer(),
                registry.offset(reg::xform_channel::X_POSITION),
                BUFFER_INDEX_X_POSITIONS);
        encoder_->setBuffer(registry.xform_buffer(),
                registry.offset(reg::xform_channel::Y_POSITION),
                BUFFER_INDEX_Y_POSITIONS);
        encoder_->setBuffer(registry.xform_buffer(),
                registry.offset(reg::xform_channel::Z_POSITION),
                BUFFER_INDEX_Z_POSITIONS);

        encoder_->setBuffer(registry.index_buffer(),
                registry.offset(reg::index_channel::SPRITE32_INDEX),
                BUFFER_INDEX_ATLAS_INDICES);
        encoder_->setBuffer(registry.index_buffer(),
                registry.offset(reg::index_channel::DRAW_ORDER),
                BUFFER_INDEX_DRAW_ORDER);

        const auto count{static_cast<std::uint32_t>(registry.count())};
        encoder_->setBytes(&count, sizeof(count), BUFFER_INDEX_ENTITY_COUNT);

        const MTL::Size grid_size{display::kWidth, display::kHeight, 1U};
        const MTL::Size thread_group_size{16U, 16U, 1U};

        encoder_->dispatchThreads(grid_size, thread_group_size);
    }

    // Mutators

    void metal_bridge::set_atlas_buffer(
            const core::mapped_view<assets::atlas>& atlas)
    {
        const auto meta{atlas->meta};

        constexpr std::size_t metadata_size{sizeof(meta)};
        const std::size_t palette_span_size{
                sizeof(graphics::palette) * meta.palette_count};
        const std::size_t sprite16_span_size{
                sizeof(assets::sprite16) * meta.sprite16_count};
        const std::size_t sprite32_span_size{
                sizeof(assets::sprite32) * meta.sprite32_count};
        const std::size_t total_size{sizeof(atlas) + palette_span_size +
                sprite16_span_size + sprite32_span_size};

        sprite32_buffer_ = NS::TransferPtr(device_->newBuffer(atlas.data(),
                total_size, MTL::ResourceStorageModeShared, nullptr));
        if (!sprite32_buffer_) [[unlikely]] {
            std::cerr << "Metal buffer is empty!\n";
            throw;
        }

        palette_span_offset_ = metadata_size;
        sprite32_span_offset_ =
                metadata_size + palette_span_size + sprite16_span_size;
    }

} // namespace sc::render
