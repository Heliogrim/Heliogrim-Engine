#pragma once

#include "../ReflowAlignment.hpp"
#include "../ReflowSpacing.hpp"

#include "FlexLine.hpp"

namespace hg::engine::reflow::algorithm {
    enum class FlexLineOrientation : bool {
        eHorizontal = true,
        eVertical = false,
    };

    struct FlexBox {
        FlexLineOrientation orientation;

        ReflowSpacing justify;
        ReflowAlignment align;

        math::vec2 gap;

        math::vec2 maxSize;
        math::vec2 preservedSize;

        bool wrap;
    };
}
