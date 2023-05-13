#pragma once

#include <Engine.GFX/Color/Color.hpp>

#include "Style.hpp"

namespace hg::engine::reflow {
    struct WindowStyle :
        public Style {
        engine::color backgroundColor { 46.F, 50.F, 58.F, 255.F };
        engine::color borderColor { 255.F, 255.F, 255.F, 255.F };
    };
}
