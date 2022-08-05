
#include "app.hpp"

#include <spdlog/spdlog.h>

#include <iostream>

int main() {

#ifdef NDEBUG
    spdlog::set_level(spdlog::level::info);
#else
    spdlog::set_level(spdlog::level::debug);
#endif

    try {
        app::run();
    } catch (const std::exception& e) {
        spdlog::error(e.what());
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}