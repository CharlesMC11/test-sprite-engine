#include "registry/entity_registry.hh"

#include <cstddef>
#include <format>
#include <iostream>

namespace sc {

    void entity_registry::print() const
    {
        for (std::size_t i{0u}; i < count(); ++i) {
            std::cout << std::format(
                    "Entity {} (Sprite Index: {})\n\tpos ({:7.2f}, {:7.2f}, "
                    "{:7.2f})\n\tvec <{:7.2f}, {:7.2f}, {:7.2f}>\n\n",
                    i, static_cast<core::index_t>(sprite32_index_ptr()[i]),
                    pos_x_ptr()[i], pos_y_ptr()[i], pos_z_ptr()[i],
                    vec_x_ptr()[i], vec_y_ptr()[i], vec_z_ptr()[i]);
        }
    }

} // namespace sc
