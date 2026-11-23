#pragma once

#include <Engine.Assets.Type/Font/GlyphRanges.hpp>

#include "__fwd.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::graphics::loader {
    struct FontLoadOptions {
        Vector<reflow::GlyphRange> ranges;
        Vector<reflow::GlyphCode> glyphs;

        /**/

        Vector<u32> fontSizes;
    };
}
