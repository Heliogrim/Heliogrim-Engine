#pragma once
#include <Engine.GFX/Color/Color.hpp>

#include "Style.hpp"

namespace hg::engine::reflow {
    struct ButtonStyle :
        public Style {
        engine::color defaultColor { 46.F, 50.F, 58.F, 255.F };
        engine::color hoverColor { 53.F, 57.F, 65.F, 255.F };
        engine::color disabledColor { 32.F, 36.F, 44.F, 255.F };
    };
}
