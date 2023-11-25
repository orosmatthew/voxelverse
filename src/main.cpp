#include <filesystem>

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

    if (!std::filesystem::exists("save")) {
        const bool result = std::filesystem::create_directory("save");
        VV_REL_ASSERT(result, "[Main] Failed to create save dir")
    }

    //    try {
    app::App instance;
    instance.main_loop();
    //    }
    //    catch (const std::exception& e) {
    //        LOG->error(e.what());
    //        return EXIT_FAILURE;
    //    }
    return EXIT_SUCCESS;
}