#pragma once

#include <Engine.Common/Math/Extent.hpp>

namespace ember::engine::gfx::glow::ui {
    struct UIContext {
        math::fExtent2D viewport;
        math::fExtent2D scissor;
    };
}
