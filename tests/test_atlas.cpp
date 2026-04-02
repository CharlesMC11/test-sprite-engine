#include <format>
#include <iostream>

#include "assets/atlas.hh"
#include "core/mapped_view.hh"

int main(const int argc, const char* argv[])
{
    if (argc < 2) [[unlikely]] {
        std::cerr << "ERROR: Must enter path to atlas!\n";
        return 1;
    }

    const sc::core::mapped_view<sc::assets::atlas> view{argv[1UZ]};
    if (!view) [[unlikely]] {
        std::cerr << "ERROR: Could not load sprites\n";
        return 1;
    }

    std::cout << view;
}
