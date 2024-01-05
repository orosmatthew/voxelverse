#include "logger.hpp"

#include <spdlog/sinks/stdout_color_sinks.h>

static bool g_init = false;

void init_logger()
{
    if (g_init) {
        return;
    }
    std::vector<spdlog::sink_ptr> sinks;
    sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
    //    sinks.push_back(std::make_shared<spdlog::sinks::daily_file_sink_mt>("logs/log.txt", 0, 0, false, 5));
    const auto logger = std::make_shared<spdlog::logger>("main", std::begin(sinks), std::end(sinks));
    register_logger(logger);
    g_init = true;
}
