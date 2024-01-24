#include "Flex.hpp"

#include <Engine.Common/Math/Coordinates.hpp>

#include "../Widget/Widget.hpp"
#include "../ReflowState.hpp"
#include "../Children.hpp"

using namespace hg::engine::reflow::algorithm;
using namespace hg::engine::reflow;
using namespace hg;

/**/

static auto addFlexLine(ref<FlexState> state_) {
    return state_.lines.insert(state_.lines.end(), FlexLine {});
}

static math::vec2 getLineTargetLimit(cref<FlexState> state_) noexcept {
    if (state_.box.orientation == FlexLineOrientation::eHorizontal) {
        return math::vec2 { state_.box.maxSize.x, -1.F };
    }
    return math::vec2 { -1.F, state_.box.maxSize.y };
}

static math::vec2 getLineShrinkAvail(cref<FlexLine> line_) noexcept {
    math::vec2 resizeAvailable { 0.F };
    for (const auto& item : line_.items) {
        resizeAvailable += item.shrinkDiff();
    }
    return resizeAvailable;
}

static math::vec2 getLineGrowAvail(ref<FlexLine> line_) noexcept {
    math::vec2 resizeAvailable { 0.F };
    for (const auto& item : line_.items) {
        resizeAvailable += item.growDiff();
    }
    return resizeAvailable;
}

static math::vec2 expandLineOriented(
    cref<math::vec2> lineSize_,
    FlexLineOrientation orientation_,
    cref<math::vec2> addition_,
    cref<math::vec2> gap_
) {

    if (orientation_ == FlexLineOrientation::eHorizontal) {
        return math::vec2 {
            lineSize_.x + addition_.x + gap_.x,
            MAX(lineSize_.y, addition_.y)
        };
    }

    return math::vec2 {
        MAX(lineSize_.x, addition_.x),
        lineSize_.y + addition_.y + gap_.y
    };
}

static void advanceOffset(FlexLineOrientation orientation_, cref<math::vec2> value_, _Inout_ ref<math::vec2> offset_) {
    if (orientation_ == FlexLineOrientation::eHorizontal) {
        offset_.x += value_.x;
    } else {
        offset_.y += value_.y;
    }
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

static FlexLine solvePreserve(
    ref<FlexState> state_,
    cref<ReflowState> reflowState_,
    const ptr<const Children> widgets_
) {

    /**
     * TODO: Solve with present state or use box a limit to check whether we got effective changes
     */
    FlexLine preserve {
        math::vec2 {},
        state_.box.preservedSize,
        1.F,
        {}
    };

    preserve.items.reserve(widgets_->size());
    for (const auto& widget : *widgets_) {

        if (widget->position() == ReflowPosition::eAbsolute) {
            continue;
        }

        constexpr auto inf = math::vec2 { std::numeric_limits<float>::infinity() };
        constexpr auto zero = math::vec2 { 0.F };

        const auto widgetState = reflowState_.getStateOf(widget);
        preserve.items.emplace_back(
            FlexLineItem {
                .widget = widget,
                .maxSize = inf,
                .baseSize = widgetState->cachedPreservedSize,
                .minSize = zero,
                .shrinkFactor = widget->shrinkFactor(),
                .growFactor = widget->growFactor(),
                .flexSize = widgetState->cachedPreservedSize,
                .frozen = false/* frozen ?!? */
            }
        );
    }

    return preserve;
}

static void solveLine(
    ref<FlexLine> line_,
    FlexLineOrientation orientation_,
    cref<math::vec2> gap_,
    cref<math::vec2> lineLimit_,
    _Inout_ ref<FlexLineItem> item_
) {

    const bool isFirst = line_.items.empty();

    item_.offset = line_.offset;

    if (isFirst) {
        advanceOffset(orientation_, line_.size, item_.offset);
    } else {
        advanceOffset(orientation_, line_.size + gap_, item_.offset);
    }

    item_.flexSize = item_.baseSize;

    /**/

    line_.items.push_back(item_);

    /**/

    if (isFirst) {
        line_.size = expandLineOriented(line_.size, orientation_, item_.baseSize, math::vec2_zero);
    } else {
        line_.size = expandLineOriented(line_.size, orientation_, item_.baseSize, gap_);
    }

    /**/

    if (
        (lineLimit_.x >= 0.F && line_.size.x > lineLimit_.x) ||
        (lineLimit_.y >= 0.F && line_.size.y > lineLimit_.y)
    ) {

        /* Restore previous unbound line size */

        const auto inverse = 1.0F - line_.flexFactor;
        const auto unbound = line_.size + line_.size * inverse;

        /* Calculate size over limit to compensate */

        const math::vec2 calcLimit = math::compMax<float>(lineLimit_, math::vec2 { 0.F });
        const math::vec2 overLimit = unbound - calcLimit;

        /* Aggregate the shrinkable size and determine the apply factor */

        const math::vec2 resizeAvailable = getLineShrinkAvail(line_);
        if (
            (orientation_ == FlexLineOrientation::eHorizontal && resizeAvailable.x == 0.F) ||
            (orientation_ == FlexLineOrientation::eVertical && resizeAvailable.y == 0.F)
        ) {
            return;
        }

        math::vec2 applyShrinkFactor { 0.F };
        applyShrinkFactor = overLimit / resizeAvailable;

        /**/

        applyShrinkFactor = math::compMin<float>(applyShrinkFactor, math::vec2 { 1.F });
        applyShrinkFactor = math::compMax<float>(applyShrinkFactor, math::vec2 { 0.F });

        /* Correct the factor based on the orientation */

        if (orientation_ == FlexLineOrientation::eHorizontal) {
            applyShrinkFactor.y = 0.F;
        } else {
            applyShrinkFactor.x = 0.F;
        }

        /* Apply the new shrinking factor to all items and aggregate new line metrics */

        const auto itemCount = line_.items.size();
        math::vec2 offset { line_.offset };
        math::vec2 size { 0.F };

        for (size_t i = 0; i < itemCount; ++i) {

            auto& item = line_.items[i];
            item.offset = offset;
            item.flexSize = item.baseSize - item.shrinkDiff() * applyShrinkFactor;

            advanceOffset(orientation_, item.flexSize + gap_, offset);

            if (i == 0) {
                size = expandLineOriented(size, orientation_, item.flexSize, math::vec2_zero);
            } else {
                size = expandLineOriented(size, orientation_, item.flexSize, gap_);
            }
        }

        /**/

        line_.size = size;
        line_.flexFactor = (orientation_ == FlexLineOrientation::eHorizontal) ?
                               (1.0F - applyShrinkFactor.x) :
                               (1.0F - applyShrinkFactor.y);
    }

    if (lineLimit_.x >= 0.F && line_.size.x < lineLimit_.x ||
        lineLimit_.y >= 0.F && line_.size.y < lineLimit_.y
    ) {

        /* Calculate size under limit to distribute */

        const math::vec2 calcLimit = math::compMax<float>(lineLimit_, math::vec2 { 0.F });
        const math::vec2 underLimit = calcLimit - line_.size;

        /* Aggregate the growable size and determine the apply factor */

        const math::vec2 resizeAvailable = getLineGrowAvail(line_);
        if (
            (orientation_ == FlexLineOrientation::eHorizontal && resizeAvailable.x == 0.F) ||
            (orientation_ == FlexLineOrientation::eVertical && resizeAvailable.y == 0.F)
        ) {
            return;
        }

        math::vec2 applyGrowFactor { 0.F };
        applyGrowFactor = underLimit / resizeAvailable;

        /**/

        applyGrowFactor = math::compMin<float>(applyGrowFactor, math::vec2 { 1.F });
        applyGrowFactor = math::compMax<float>(applyGrowFactor, math::vec2 { 0.F });

        /* Correct the factor based on the orientation */

        if (orientation_ == FlexLineOrientation::eHorizontal) {
            applyGrowFactor.y = 0.F;
        } else {
            applyGrowFactor.x = 0.F;
        }

        /* Apply the new grow factor to all items and aggregate new line metrics */

        const auto itemCount = line_.items.size();
        math::vec2 offset { line_.offset };
        math::vec2 size { 0.F };

        for (size_t i = 0; i < itemCount; ++i) {

            auto& item = line_.items[i];
            item.offset = offset;
            item.flexSize = item.baseSize + item.growDiff() * applyGrowFactor;

            advanceOffset(orientation_, item.flexSize + gap_, offset);

            if (i == 0) {
                size = expandLineOriented(size, orientation_, item.flexSize, math::vec2_zero);
            } else {
                size = expandLineOriented(size, orientation_, item.flexSize, gap_);
            }
        }

        /**/

        line_.size = size;
        line_.flexFactor = (orientation_ == FlexLineOrientation::eHorizontal) ?
                               (1.0F + applyGrowFactor.x) :
                               (1.0F + applyGrowFactor.y);
    }
}

static math::vec2 arrangeCrossAxis(
    cref<FlexBox> box_,
    cref<math::vec2> lineLimit_,
    cref<FlexLineItem> lineItem_,
    cref<math::vec2> mainAxisOffset_
) {

    const auto left = lineLimit_ - lineItem_.flexSize;

    math::vec2 align { 0.F };
    if (box_.orientation == FlexLineOrientation::eVertical) {
        align = math::vec2 { MAX(left.x, 0.F), 0.F };
    } else {
        align = math::vec2 { 0.F, MAX(left.y, 0.F) };
    }

    math::vec2 alignBefore { 0.F };
    switch (box_.align) {
        case ReflowAlignment::eStart: {
            break;
        }
        case ReflowAlignment::eCenter: {
            alignBefore = align / 2.F;
            break;
        }
        case ReflowAlignment::eEnd: {
            alignBefore = align;
            break;
        }
    }

    return mainAxisOffset_ + alignBefore;
}

static void arrangeMainAxis(
    cref<FlexBox> box_,
    cref<math::vec2> lineLimit_,
    ref<FlexLine> line_
) {

    if (line_.items.empty()) {
        return;
    }

    const auto left = lineLimit_ - line_.size;

    math::vec2 justify { 0.F };
    if (box_.orientation == FlexLineOrientation::eVertical) {
        justify = math::vec2 { 0.F, MAX(left.y, 0.F) };
    } else {
        justify = math::vec2 { MAX(left.x, 0.F), 0.F };
    }

    math::vec2 justifyBefore { 0.F };
    math::vec2 justifyBetween { 0.F };
    math::vec2 justifyAfter { 0.F };

    switch (box_.justify) {
        case ReflowSpacing::eSpaceAround: {
            justifyBefore = justify / 2.F;
            justifyAfter = justify - justifyBefore;
            break;
        }
        case ReflowSpacing::eSpaceBetween: {
            justifyBetween = justify / static_cast<float>(line_.items.size() - 1);
            break;
        }
        case ReflowSpacing::eSpaceEvenly: {
            justifyBetween = justify / static_cast<float>(line_.items.size() + 1);
            justifyBefore = justifyBetween;
            justifyAfter = justifyBetween;
            break;
        }
        case ReflowSpacing::eStart: {
            justifyAfter = justify;
            break;
        }
        case ReflowSpacing::eEnd: {
            justifyBefore = justify;
            break;
        }
    }

    /**/

    if (box_.orientation == FlexLineOrientation::eVertical) {
        justifyBetween.y += box_.gap.y;
    } else {
        justifyBetween.x += box_.gap.x;
    }

    /**/

    const auto lastItem = line_.items.size() - 1;
    math::vec2 offset = line_.offset;

    for (size_t i = 0; i <= lastItem; ++i) {

        if (i == 0) {
            offset += justifyBefore;
        }

        line_.items[i].offset = arrangeCrossAxis(box_, /*lineLimit_*/line_.size, line_.items[i], offset);
        advanceOffset(box_.orientation, line_.items[i].flexSize, offset);

        if (i == lastItem) {
            offset += justifyAfter;
        } else {
            offset += justifyBetween;
        }
    }
}

ref<FlexState> algorithm::solve(
    /* TODO: __restricted_ptr<FlexState> */ref<FlexState> state_,
    cref<ReflowState> reflowState_,
    const ptr<const Children> widgets_
) {

    const bool isUpdate = not state_.lines.empty();
    const auto lineLimit = getLineTargetLimit(state_);

    /**/

    auto preserved = solvePreserve(state_, reflowState_, widgets_);

    /**/

    auto line = state_.lines.begin();
    if (state_.lines.empty()) {
        line = addFlexLine(state_);
    }

    for (auto& preservedItem : preserved.items) {

        // TODO: Check whether preserve <=> state stored line(-item) changed
        const math::vec2 bb = preservedItem.baseSize;
        //const math::vec2 bb = preservedItem.minSize;

        if (willOverflowLine(*line, state_.box.orientation, lineLimit, bb) && shouldWrapLine(state_)) {

            if ((++line) == state_.lines.end()) {
                arrangeMainAxis(state_.box, lineLimit, *line);

                auto nextLineOff = line->offset;
                advanceOffset(state_.box.orientation, line->size + state_.box.gap, nextLineOff);

                line = addFlexLine(state_);
                line->offset = nextLineOff;
            }
        }

        // TODO: Only (re-)solve flex line if line state or stored items state changed
        solveLine(*line, state_.box.orientation, state_.box.gap, lineLimit, preservedItem);
    }

    arrangeMainAxis(state_.box, lineLimit, *line);
    return state_;
}
