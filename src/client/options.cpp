#include "options.hpp"

#include <fstream>

#include <nlohmann/json.hpp>

#include "common.hpp"
#include "logger.hpp"

using json = nlohmann::json;

Options load_options()
{
    std::ifstream file("options.json");
    if (!file.is_open()) {
        return Options {};
    }
    const json data = json::parse(file, nullptr, false);
    if (data.is_discarded()) {
        LOG->error("[Options] Invalid JSON");
        return Options {};
    }
    Options options;
    if (data.contains("fullscreen") && data["fullscreen"].is_boolean()) {
        options.fullscreen = data["fullscreen"].get<bool>();
    }
    if (data.contains("msaa") && data["msaa"].is_number_integer()) {
        switch (data["msaa"].get<int>()) {
        case 1:
            options.msaa = mve::Msaa::samples_1;
            break;
        case 2:
            options.msaa = mve::Msaa::samples_2;
            break;
        case 4:
            options.msaa = mve::Msaa::samples_4;
            break;
        case 8:
            options.msaa = mve::Msaa::samples_8;
            break;
        case 16:
            options.msaa = mve::Msaa::samples_16;
            break;
        case 32:
            options.msaa = mve::Msaa::samples_32;
            break;
        case 64:
            options.msaa = mve::Msaa::samples_64;
            break;
        default:
            break;
        }
    }
    return options;
}

void set_options(const Options& options)
{
    auto msaa_int = [](const mve::Msaa msaa) {
        switch (msaa) {
        case mve::Msaa::samples_1:
            return 1;
        case mve::Msaa::samples_2:
            return 2;
        case mve::Msaa::samples_4:
            return 4;
        case mve::Msaa::samples_8:
            return 8;
        case mve::Msaa::samples_16:
            return 16;
        case mve::Msaa::samples_32:
            return 32;
        case mve::Msaa::samples_64:
            return 64;
        }
        VV_REL_ASSERT(false, "[Options] Unreachable: Invalid MSAA");
    };

    std::ofstream file("options.json");
    const json data = { { "fullscreen", options.fullscreen }, { "msaa", msaa_int(options.msaa) } };
    file << std::setw(4) << data << std::endl;
}