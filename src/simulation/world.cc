#include "world.hh"

#include <iostream>

#include "assets/asset_constants.hh"
#include "graphics/display_constants.hh"
#include "simulation/physics.hh"
#include "simulation/physics_types.hh"

namespace sc {

    [[nodiscard]] world::world(MTL::Device* device)
        : grid_{}, bridge_{render::metal_bridge{device}},
          registry_{entity_registry{device}},
          atlas_{core::mapped_view<assets::atlas>{assets::kAtlas}}
    {
        if (!(atlas_ && atlas_.data())) {
            std::cerr << "FATAL: Could not load the atlas file!\n";
            throw;
        }
        if (!assets::atlas::validate(atlas_.data(), atlas_.size())) {
            std::cerr << "FATAL: Atlas has an invalid header or size!\n";
            throw;
        }

        bridge_.set_atlas_buffer(atlas_);

        constexpr auto id{assets::sprite32_index::LANCIS};
        const assets::sprites::metadata& sprite{atlas_->operator[](id).meta};
        registry_.spawn((display::kWidth - 32U - sprite.origin_u) * 0.5f,
                (display::kHeight - 32U - sprite.origin_v) * 0.5f, 0.0f, id);

        registry_.spawn(0.0f, 0.0f, 0.0f, assets::sprite32_index::MYARRA);

        registry_.spawn(display::kWidth * 0.75f, display::kHeight * 0.75f,
                26.0f, assets::sprite32_index::HEART_OW_F);
    }

    void world::update(const input::mask input, const MTL::Drawable* drawable)
    {
        float frame_time{1.0f / display::kTargetFPS};
        if (frame_time > 0.25f)
            frame_time = 0.25f;

        ftime_accumulator_ += frame_time;

        while (ftime_accumulator_ >= physics::kFixedTimestep) {
            constexpr float speed{200.0f};
            grid_.update(registry_);

            registry_.vel_x_ptr()[0UZ] = registry_.vel_y_ptr()[0UZ] = 0.0f;
            if (core::any(input & input::mask::UP))
                registry_.vel_y_ptr()[0UZ] -= speed;
            if (core::any(input & input::mask::DOWN))
                registry_.vel_y_ptr()[0UZ] += speed;
            if (core::any(input & input::mask::LEFT))
                registry_.vel_x_ptr()[0UZ] -= speed;
            if (core::any(input & input::mask::RIGHT))
                registry_.vel_x_ptr()[0UZ] += speed;

            physics::resolve_entity_collisions(
                    registry_, grid_, *atlas_.data(), physics::kFixedTimestep);

            registry_.commit();
            ftime_accumulator_ -= physics::kFixedTimestep;
        }

        registry_.draw_order_needs_sort = true;
        registry_.sort_draw();

        bridge_.begin_frame(drawable);
        bridge_.clear();
        bridge_.draw(registry_);
        bridge_.end_frame(drawable);
    }

} // namespace sc
