#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::reflow {
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
