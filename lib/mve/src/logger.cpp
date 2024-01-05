#include "logger.hpp"

#include <vector>

#include <spdlog/sinks/stdout_color_sinks.h>

std::optional<spdlog::logger> g_logger;

static void init_logger()
{
    std::vector<spdlog::sink_ptr> sinks;
    sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
    g_logger = spdlog::logger("MVE", std::begin(sinks), std::end(sinks));
}

spdlog::logger& log()
{
    if (!g_logger.has_value()) {
        init_logger();
    }
    return g_logger.value();
}