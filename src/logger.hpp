#pragma once

#include <spdlog/spdlog.h>

#define LOG spdlog::get("main")

void initLogger();
