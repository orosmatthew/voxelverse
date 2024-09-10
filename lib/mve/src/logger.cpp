#include "logger.hpp"

namespace mve {

std::mutex Logger::m_mutex;
std::optional<std::unique_ptr<Logger>> Logger::m_instance;

}