#include <iostream>

#include <spdlog/spdlog.h>

#include "app.hpp"
#include "logger.hpp"

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
        app::run();
    }
    catch (const std::exception &e) {
        LOG->error(e.what());
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}