#pragma once

#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <string>

namespace mve {

enum class LogLevel { debug = 0, info = 1, warning = 2, error = 3 };

class Logger {
public:
    static void log(LogLevel level, const std::string& message)
    {
        if (Logger& logger = get(); static_cast<int>(level) >= static_cast<int>(logger.m_level)) {
            if (level == LogLevel::error) {
                std::cerr << level_to_string(level) << " [MVE] " << message << std::endl;
            }
            else {
                std::cout << level_to_string(level) << " [MVE] " << message << std::endl;
            }
        }
    }

    static void debug(const std::string& message)
    {
        log(LogLevel::debug, message);
    }

    static void info(const std::string& message)
    {
        log(LogLevel::info, message);
    }

    static void warning(const std::string& message)
    {
        log(LogLevel::warning, message);
    }

    static void error(const std::string& message)
    {
        log(LogLevel::error, message);
    }

    static void set_log_level(const LogLevel level)
    {
        Logger& logger = get();
        logger.m_level = level;
    }

    static LogLevel log_level()
    {
        const Logger& logger = get();
        return logger.m_level;
    }

private:
    Logger() = default;

    static Logger& get()
    {
        std::lock_guard lock(m_mutex);
        if (!m_instance.has_value()) {
            m_instance = std::unique_ptr<Logger>(new Logger());
        }
        return **m_instance;
    }

    static std::string level_to_string(const LogLevel level)
    {
        switch (level) {
        case LogLevel::debug:
            return "[Debug]";
        case LogLevel::info:
            return "[Info]";
        case LogLevel::warning:
            return "[Warning]";
        case LogLevel::error:
            return "[Error]";
        }
        return "[Unknown]";
    }

    LogLevel m_level = LogLevel::info;
    static std::mutex m_mutex;
    static std::optional<std::unique_ptr<Logger>> m_instance;
};

}
