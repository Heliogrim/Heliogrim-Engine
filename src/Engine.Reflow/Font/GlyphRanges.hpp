#pragma once

#include <Engine.Common/Types.hpp>

#include "GlyphCode.hpp"

namespace ember::engine::reflow {

    struct GlyphRange {
        GlyphCode begin;
        GlyphCode end;
    };

    constexpr GlyphRange BasicLatin { 0x0020, 0x007F };
    constexpr GlyphRange Latin1Supplement { 0x00A0, 0x00FF };
    constexpr GlyphRange LatinExtA { 0x0100, 0x017F };
    constexpr GlyphRange LatinExtB { 0x0180, 0x024F };

    /**/

    constexpr GlyphRange Spacing { 0x02B0, 0x02FF };

    /**/

    constexpr GlyphRange NumberForms { 0x2150, 0x218F };
    constexpr GlyphRange Arrows { 0x2190, 0x21FF };
    constexpr GlyphRange MathOps { 0x2200, 0x22FF };
    constexpr GlyphRange MiscTech { 0x2300, 0x23FF };

}
