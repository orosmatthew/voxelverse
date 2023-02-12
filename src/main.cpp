#include <iostream>

#include "app.hpp"
#include "logger.hpp"

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