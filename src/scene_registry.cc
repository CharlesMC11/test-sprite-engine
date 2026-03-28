#include "scene_registry.hh"

#include <cstddef>
#include <cstdio>

namespace sc {

    void scene_registry::print() const noexcept
    {
        for (std::size_t i{0u}; i < count(); ++i) {
            std::printf("Entity %zu (Sprite Index: %u)\n\tpos (%7.2f, %7.2f, "
                        "%7.2f)\n\tvec <%7.2f, %7.2f, %7.2f>\n\n",
                    i, static_cast<core::index_t>(indices[i]), pos_x()[i],
                    pos_y()[i], pos_z()[i], vec_x()[i], vec_y()[i], vec_z()[i]);
        }
    }

} // namespace sc
