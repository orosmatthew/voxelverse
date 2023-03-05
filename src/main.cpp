#include <iostream>

#include "app.hpp"
#include "logger.hpp"
#include "mve/common.hpp"
#include <filesystem>

#include <spdlog/spdlog.h>

int main()
{
    initLogger();

#ifdef NDEBUG
    LOG->set_level(spdlog::level::info);
#else
    LOG->set_level(spdlog::level::debug);
#endif

    LOG->info("Starting");

    if (!std::filesystem::exists("save")) {
        bool result = std::filesystem::create_directory("save");
        MVE_ASSERT(result, "[Main] Failed to create save dir")
    }

    try {
        app::App instance;
        instance.main_loop();
    }
    catch (const std::exception& e) {
        LOG->error(e.what());
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}