#pragma once
#include <Engine.GFX/Color/Color.hpp>

#include "Style.hpp"

namespace hg::engine::reflow {
    struct SplitStyle :
        public Style {
        color defaultColor { 53.F, 57.F, 65.F, 255.F };
    };
}
