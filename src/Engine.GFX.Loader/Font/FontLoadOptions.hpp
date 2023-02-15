#pragma once

#include "__fwd.hpp"

#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Reflow/Font/GlyphRanges.hpp>

namespace ember::engine::gfx::loader {
    struct FontLoadOptions {
        Vector<reflow::GlyphRange> ranges;
        Vector<reflow::GlyphCode> glyphs;

        /**/

        Vector<u32> fontSizes;
    };
}
