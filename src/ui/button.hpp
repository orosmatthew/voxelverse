#pragma once

#include "nine_patch.hpp"

class Button {
public:
    explicit Button(UIPipeline& pipeline, float scale = 1.0f);

    void draw() const;

private:
    NinePatch m_patch;
};