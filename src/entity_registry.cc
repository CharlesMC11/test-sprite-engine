#include <cstddef>
#include <cstdio>

#include "entity_registry.hh"

namespace sc {

    void entity_registry::print() const noexcept
    {
        for (std::size_t i{0u}; i < count(); ++i) {
            std::printf("Entity %zu (Sprite Index: %u)\n\tpos (%7.2f, %7.2f, "
                        "%7.2f)\n\tvec <%7.2f, %7.2f, %7.2f>\n\n",
                    i, static_cast<core::index_t>(indices[i]), pos_x_ptr()[i],
                    pos_y_ptr()[i], pos_z_ptr()[i], vec_x_ptr()[i], vec_y_ptr()[i], vec_z_ptr()[i]);
        }
    }

} // namespace sc
