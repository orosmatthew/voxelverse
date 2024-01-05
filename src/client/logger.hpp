#pragma once

// ReSharper disable once CppUnusedIncludeDirective
#include <spdlog/spdlog.h>

/**
 * @brief macro to main logger
 */
#define LOG spdlog::get("main")

/**
 * @brief Initialize logger
 */
void initLogger();
