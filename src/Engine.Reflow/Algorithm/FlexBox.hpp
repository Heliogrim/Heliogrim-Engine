#pragma once

#include "FlexLine.hpp"

namespace hg::engine::reflow::algorithm {
    enum class FlexLineOrientation : bool {
        eHorizontal = true,
        eVertical = false,
    };

    struct FlexBox {
        FlexLineOrientation orientation;

        math::vec2 maxSize;
        math::vec2 preservedSize;

        bool wrap;
    };
}
