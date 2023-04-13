#pragma once

#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Math/Vector.hpp>

namespace hg::engine::reflow::algorithm {
    struct FlexLineItem {
        wptr<void> widget;

        math::vec2 offset;

        math::vec2 maxSize;
        math::vec2 baseSize;
        math::vec2 minSize;

        float shrinkFactor;
        float growFactor;

        math::vec2 flexSize;

        bool frozen;

    public:
        [[nodiscard]] math::vec2 shrinkDiff() const noexcept {
            if (shrinkFactor <= 0.F || minSize.x < 0.F || minSize.y < 0.F) {
                return math::vec2 { 0.F };
            }
            return math::vec2 { baseSize - minSize };
        }

        [[nodiscard]] math::vec2 growDiff() const noexcept {
            constexpr auto inf = _STD numeric_limits<float>::infinity();
            if (growFactor <= 0.F || maxSize.x < 0.F || maxSize.y < 0.F || maxSize.x >= inf || maxSize.y >= inf) {
                return math::vec2 { 0.F };
            }
            return math::vec2 { maxSize - baseSize };
        }
    };

    struct FlexLine {
        math::vec2 offset;
        math::vec2 size;

        float flexFactor;

        Vector<FlexLineItem> items;
    };
}
