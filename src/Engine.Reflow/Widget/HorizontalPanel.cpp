#include "HorizontalPanel.hpp"

#include <Engine.Logging/Logger.hpp>

#include "../Algorithm/Flex.hpp"
#include "../Layout/Style.hpp"
#include "../Children.hpp"

using namespace hg::engine::reflow;
using namespace hg;

HorizontalPanel::HorizontalPanel() :
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

HorizontalPanel::~HorizontalPanel() = default;

string HorizontalPanel::getTag() const noexcept {
    return std::format(R"(HorizontalPanel <{:#x}>)", reinterpret_cast<u64>(this));
}

const ptr<const Children> HorizontalPanel::children() const {
    return &_children;
}

void HorizontalPanel::addChild(cref<sptr<Widget>> child_) {

    child_->setParent(shared_from_this());
    _children.push_back(child_);

    markAsPending();
}

void HorizontalPanel::setChild(const u32 idx_, cref<sptr<Widget>> child_) {

    if (idx_ == _children.size()) {
        addChild(child_);
    }

    if (idx_ > _children.size()) {
        return;
    }

    child_->setParent(shared_from_this());
    _children[idx_]->setParent(nullptr);
    _children[idx_] = child_;

    markAsPending();
}

void HorizontalPanel::removeChild(cref<sptr<Widget>> child_) {

    s32 found = -1L;
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

void HorizontalPanel::clearChildren() {

    for (const auto& child : _children) {
        child->setParent(nullptr);
    }

    _children.clear();

    markAsPending();
}

void HorizontalPanel::render(const ptr<ReflowCommandBuffer> cmd_) {
    Panel::renderPanel(cmd_, attr.style.getValue());
    Panel::render(cmd_);
}

math::vec2 HorizontalPanel::prefetchDesiredSize(cref<ReflowState> state_, float scale_) const {

    math::vec2 childAggregate;
    math::vec2 childMax;

    for (const auto& child : *children()) {

        if (child->position() == ReflowPosition::eAbsolute) {
            continue;
        }

        const auto* const childState = state_.getStateOf(child);

        childAggregate += child->getDesiredSize();
        childMax = math::compMax<float>(childMax, child->getDesiredSize());
    }

    const auto gapping = math::vec2 { attr.colGap.getValue(), attr.rowGap.getValue() } *
        static_cast<float>(MAX(children()->size(), 1) - 1);

    /**/

    const math::vec2 innerSize = math::vec2 { childAggregate.x + gapping.x, childMax.y };
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

math::vec2 HorizontalPanel::computeDesiredSize(cref<ReflowPassState> passState_) const {

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

void HorizontalPanel::applyLayout(ref<ReflowState> state_, mref<LayoutContext> ctx_) {

    const auto innerSize = layout::outerToInnerSize(attr, ctx_.localSize);

    algorithm::FlexState flexState {};
    flexState.box.preservedSize = innerSize;
    flexState.box.maxSize = innerSize;
    flexState.box.orientation = algorithm::FlexLineOrientation::eHorizontal;
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
            const auto widgetState = state_.getStateOf(std::static_pointer_cast<Widget, void>(dummy));

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

    /**/

    // Warning: Hotfix
    for (const auto& child : _children) {
        if (child->position() == ReflowPosition::eAbsolute) {

            const auto widgetState = state_.getStateOf(child);
            widgetState->layoutOffset = ctx_.localOffset;
            widgetState->layoutSize = widgetState->cachedPreservedSize;
        }
    }
}

float HorizontalPanel::shrinkFactor() const noexcept {
    return attr.flexShrink.getValue();
}

float HorizontalPanel::growFactor() const noexcept {
    return attr.flexGrow.getValue();
}
