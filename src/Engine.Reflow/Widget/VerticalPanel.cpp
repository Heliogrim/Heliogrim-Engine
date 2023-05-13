#include "VerticalPanel.hpp"

#include <Engine.Logging/Logger.hpp>
#include "../Algorithm/Flex.hpp"
#include "../Algorithm/FlexState.hpp"
#include "../Layout/Style.hpp"
#include "../Children.hpp"
#include "../Style/PanelStyle.hpp"

using namespace hg::engine::reflow;
using namespace hg;

VerticalPanel::VerticalPanel():
    Panel(),
    attr(
        Attributes {
            .minWidth = { this, { ReflowUnitType::eAuto, 0.F } },
            .width = { this, { ReflowUnitType::eAuto, 0.F } },
            .maxWidth = { this, { ReflowUnitType::eAuto, 0.F } },
            .minHeight = { this, { ReflowUnitType::eAuto, 0.F } },
            .height = { this, { ReflowUnitType::eAuto, 0.F } },
            .maxHeight = { this, { ReflowUnitType::eAuto, 0.F } },
            .padding = { this, Padding { 0.F } },
            .justify = { this, ReflowSpacing::eStart },
            .align = { this, ReflowAlignment::eStart },
            .colGap = { this, 0.F },
            .rowGap = { this, 0.F },
            .flexGrow = { this, 0.F },
            .flexShrink = { this, 0.F },
            .style = { this, PanelStyle {} }
        }
    ),
    _children() {}

VerticalPanel::~VerticalPanel() = default;

string VerticalPanel::getTag() const noexcept {
    return _STD format(R"(VerticalPanel <{:#x}>)", reinterpret_cast<u64>(this));
}

const ptr<const Children> VerticalPanel::children() const {
    return &_children;
}

void VerticalPanel::addChild(cref<sptr<Widget>> child_) {

    child_->setParent(shared_from_this());
    _children.push_back(child_);

    markAsPending();
}

void VerticalPanel::setChild(const u32 idx_, cref<sptr<Widget>> child_) {

    if (idx_ == _children.size()) {
        addChild(child_);
        return;
    }

    if (idx_ > _children.size()) {
        return;
    }

    child_->setParent(shared_from_this());
    _children[idx_]->setParent(nullptr);
    _children[idx_] = child_;
}

void VerticalPanel::removeChild(cref<sptr<Widget>> child_) {

    s32 found = -1i32;
    for (u32 idx = 0; idx < _children.size(); ++idx) {
        if (_children[idx] == child_) {
            found = idx;
            break;
        }
    }

    /**/

    if (found < 0) {
        return;
    }

    const auto iter = _children.cbegin() + found;

    (*iter)->setParent(nullptr);
    _children.erase(_children.cbegin() + found);

    markAsPending();
}

void VerticalPanel::clearChildren() {

    for (const auto& child : _children) {
        child->setParent(nullptr);
    }

    _children.clear();

    markAsPending();
}

void VerticalPanel::render(cref<ReflowState> state_, const ptr<ReflowCommandBuffer> cmd_) {
    Panel::renderPanel(state_, cmd_, attr.style.getValue());
    Panel::render(state_, cmd_);
}

math::vec2 VerticalPanel::prefetchDesiredSize(cref<ReflowState> state_, float scale_) const {

    math::vec2 childAggregate;
    math::vec2 childMax;

    for (const auto& child : *children()) {

        const auto* const childState = state_.getStateOf(child);

        childAggregate += child->getDesiredSize();
        childMax = math::compMax<float>(childMax, child->getDesiredSize());
    }

    const auto gapping = math::vec2 { attr.colGap.getValue(), attr.rowGap.getValue() }
        * static_cast<float>(MAX(children()->size(), 1) - 1);

    /**/

    const math::vec2 innerSize = math::vec2 { childMax.x, childAggregate.y + gapping.y };
    math::vec2 size = layout::innerToOuterSize(attr, innerSize);

    /**/

    if (attr.width->type == ReflowUnitType::eAbsolute) {
        size.x = attr.width->value;
    }
    if (attr.height->type == ReflowUnitType::eAbsolute) {
        size.y = attr.height->value;
    }

    /**/

    return layout::clampSizeAbs(attr, size);
}

math::vec2 VerticalPanel::computeDesiredSize(cref<ReflowPassState> passState_) const {

    math::vec2 desired { getDesiredSize() };
    if (attr.width->type == ReflowUnitType::eRelative) {
        desired.x = passState_.referenceSize.x * attr.width->value;
    }
    if (attr.height->type == ReflowUnitType::eRelative) {
        desired.y = passState_.referenceSize.y * attr.height->value;
    }

    return layout::clampSize(
        attr,
        passState_.layoutSize,
        desired
    );
}

void VerticalPanel::applyLayout(ref<ReflowState> state_, mref<LayoutContext> ctx_) {

    const auto innerSize = layout::outerToInnerSize(attr, ctx_.localSize);

    algorithm::FlexState flexState {};
    flexState.box.preservedSize = innerSize;
    flexState.box.maxSize = innerSize;
    flexState.box.orientation = algorithm::FlexLineOrientation::eVertical;
    flexState.box.justify = attr.justify.getValue();
    flexState.box.align = attr.align.getValue();
    flexState.box.gap = math::vec2 { attr.colGap.getValue(), attr.rowGap.getValue() };
    flexState.box.wrap = false;

    /**/

    algorithm::solve(flexState, state_, children());

    /**/

    const auto minFlexBound = /*ctx_.localOffset*/math::vec2 { 0.F };
    const auto maxFlexBound = /*ctx_.localOffset + ctx_.localSize*/innerSize;

    const auto offset = layout::outerToInnerOffset(attr, ctx_.localOffset);

    /**/

    for (const auto& flexLine : flexState.lines) {
        for (const auto& flexItem : flexLine.items) {

            const auto dummy = flexItem.widget.lock();
            const auto widgetState = state_.getStateOf(_STD static_pointer_cast<Widget, void>(dummy));

            widgetState->layoutOffset = flexItem.offset + offset;
            widgetState->layoutSize = flexItem.flexSize;

            // TODO: Check how we should work with co-axis
            // Currently Flex-Solving will guarantee main-axis constraint (as long as possible), but co-axis will break
            // We might try to hard-limit contextual constraints

            math::vec2 minDiff;
            math::vec2 maxDiff;

            const auto minCorner = flexItem.offset;
            const auto maxCorner = flexItem.offset + flexItem.flexSize;

            if (maxCorner.x > maxFlexBound.x || maxCorner.y > maxFlexBound.y) {
                maxDiff = maxCorner - maxFlexBound;
                maxDiff = math::compMax<float>(maxDiff, math::vec2 { 0.F });
            }

            if (minCorner.x > minFlexBound.x || minCorner.y > minFlexBound.y) {
                minDiff = minFlexBound - minCorner;
                minDiff = math::compMax<float>(minDiff, math::vec2 { 0.F });
            }

            widgetState->layoutOffset += minDiff;
            widgetState->layoutSize -= maxDiff;
        }
    }
}
