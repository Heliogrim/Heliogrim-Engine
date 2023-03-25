#pragma once

#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Math/Vector.hpp>

namespace hg::engine::reflow::algorithm {
    struct FlexLineItem {
        wptr<void> widget;

        math::vec2 size;
        math::vec2 minSize;

        bool frozen;
    };

    struct FlexLine {
        math::vec2 offset;
        math::vec2 size;

        Vector<FlexLineItem> items;
    };
}
