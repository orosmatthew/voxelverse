
#include "app.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/daily_file_sink.h>

#include <iostream>

int main() {

#ifdef NDEBUG
    spdlog::set_level(spdlog::level::info);
#else
    spdlog::set_level(spdlog::level::debug);
#endif

    auto logger = spdlog::daily_logger_mt("my_log", "logs/daily.txt", 0, 0, false, 30);

    logger->debug("YO!");

    try {
        app::run();
    } catch (const std::exception& e) {
        spdlog::error(e.what());
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}