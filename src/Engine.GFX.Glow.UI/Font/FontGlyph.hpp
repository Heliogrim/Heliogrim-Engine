#pragma once

#include <Engine.Common/Types.hpp>
#include <Engine.Common/Math/Vector.hpp>

namespace ember::engine::gfx::glow::ui {

    struct FontGlyph {
        /**/
        math::vec2 st0;
        math::vec2 st1;

        /**/
        math::vec2 xy0;
        math::vec2 xy1;

        /**/
        float advance;
    };

}