#pragma once

#include <Engine.Common/Types.hpp>
#include <Engine.Common/Math/Vector.hpp>

namespace ember::engine::reflow {
    struct FontGlyph {
        /**/
        math::uivec2 _size;
        math::ivec2 _bearing;
        float _advance;

        /**/
        math::vec2 _minSt;
        math::vec2 _maxSt;
    };
}
