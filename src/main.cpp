
#include "app.hpp"

#include <spdlog/spdlog.h>

#include <iostream>

int main() {
    try {
        app::run();
    } catch (const std::exception& e) {
        spdlog::error(e.what());
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}