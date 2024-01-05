#pragma once

#include <mve/renderer.hpp>

struct Options {
    bool fullscreen = false;
    mve::Msaa msaa = mve::Msaa::samples_1;
};

Options load_options();

void set_options(const Options& options);
