#include "Flex.hpp"

#include "../Widget/Widget.hpp"

using namespace hg::engine::reflow::algorithm;
using namespace hg::engine::reflow;
using namespace hg;

/**/

static auto addFlexLine(ref<FlexState> state_) {
    return state_.lines.insert(state_.lines.end(), {});
}

static math::vec2 getLineTargetLimit(ref<FlexState> state_) noexcept {
    if (state_.box.orientation == FlexLineOrientation::eHorizontal) {
        return math::vec2 { state_.box.maxSize.x, -1.F };
    }
    return math::vec2 { -1.F, state_.box.maxSize.y };
}

static bool willOverflowLine(
    ref<FlexLine> line_,
    FlexLineOrientation orientation_,
    cref<math::vec2> limit_,
    cref<math::vec2> test_
) noexcept {

    const math::vec2 tmpMax { line_.offset + math::compMax<float>(line_.size, test_) };
    const math::vec2 tmpAgg { test_ + line_.size };

    if (orientation_ == FlexLineOrientation::eHorizontal) {
        //return tmpAgg.x > limit_.x || tmpMax.y > limit_.y;
        return tmpAgg.x > limit_.x;
    }
    //return tmpAgg.y > limit_.y || tmpMax.x > limit_.x;
    return tmpAgg.y > limit_.y;
}

/**/

static bool shouldWrapLine(ref<FlexState> state_) {
    return state_.box.wrap;
}

/**/

static FlexLine solvePreserve(ref<FlexState> state_, _STD span<sptr<Widget>> widgets_) {

    /**
     * TODO: Solve with present state or use box a limit to check whether we got effective changes
     */
    FlexLine preserve {
        math::vec2 {},
        state_.box.preservedSize,
        {}
    };

    preserve.items.reserve(widgets_.size());
    for (const auto& widget : widgets_) {

        // TODO: Solve widget
        const math::vec2 size = widget->outerSize();
        const math::vec2 minSize = widget->outerSize();

        preserve.items.push_back({ widget, size, minSize, false/* frozen ?!? */ });
    }

    return preserve;
}

static void solveLine(ref<FlexLine> line_, cref<FlexLineItem> item_) {
    // TODO:
    line_.items.push_back(item_);
}

ref<FlexState> algorithm::solve(
    /* TODO: __restricted_ptr<FlexState> */ref<FlexState> state_,
    std::span<sptr<Widget>> widgets_
) {

    const bool isUpdate = not state_.lines.empty();
    const auto lineLimit = getLineTargetLimit(state_);

    /**/

    const auto preserved = solvePreserve(state_, widgets_);

    /**/

    auto line = state_.lines.begin();
    if (state_.lines.empty()) {
        line = addFlexLine(state_);
    }

    for (const auto& preservedItem : preserved.items) {

        // TODO: Check whether preserve <=> state stored line(-item) changed
        const math::vec2 bb = preservedItem.size;
        //const math::vec2 bb = preservedItem.minSize;

        if (willOverflowLine(*line, state_.box.orientation, lineLimit, bb) && shouldWrapLine(state_)) {

            if ((++line) == state_.lines.end()) {
                line = addFlexLine(state_);
            }
        }

        // TODO: Only (re-)solve flex line if line state or stored items state changed
        solveLine(*line, preservedItem);
    }

    return state_;
}
