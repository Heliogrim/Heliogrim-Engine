#pragma once
#include <Engine.GFX/Color/Color.hpp>

#include "Style.hpp"

namespace hg::engine::reflow {
    struct PanelStyle :
        public Style {
        color backgroundColor { 46.F, 50.F, 58.F, 255.F };
    };
}
