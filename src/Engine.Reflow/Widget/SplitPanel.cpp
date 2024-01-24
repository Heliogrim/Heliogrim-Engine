#include "SplitPanel.hpp"

#include <Engine.Logging/Logger.hpp>

#include "../Algorithm/Flex.hpp"
#include "../Layout/Style.hpp"
#include "../Children.hpp"

using namespace hg::engine::reflow;
using namespace hg;

SplitPanel::SplitPanel() :
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
            .orientation = { this, ReflowOrientation::eHorizontal },
            .flexGrow = { this, 0.F },
            .flexShrink = { this, 0.F },
            .slot = Attributes::SlotAttributes {
                .justify = { this, ReflowSpacing::eSpaceAround },
                .align = { this, ReflowAlignment::eCenter },
                .padding = { this, Padding { 0.F } }
            },
            .style = { this, SplitStyle {} }
        }
    ),
    _children(),
    _slots() {}

SplitPanel::~SplitPanel() = default;

string SplitPanel::getTag() const noexcept {
    return std::format(R"(SplitPanel <{:#x}>)", reinterpret_cast<u64>(this));
}

const ptr<const Children> SplitPanel::children() const {
    return &_children;
}

void SplitPanel::addChild(cref<sptr<Widget>> child_) {

    child_->setParent(child_);

    _children.push_back(child_);
    _slots.push_back({ -1.F });

    recomputeSlotPortions();
    markAsPending();
}

void SplitPanel::setChild(const u32 idx_, cref<sptr<Widget>> child_) {

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

void SplitPanel::removeChild(cref<sptr<Widget>> child_) {

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

    const auto slot = _slots[found];
    _slots.erase(_slots.cbegin() + found);
    recomputeSlotPortions(slot.proportion);

    /**/

    const auto iter = _children.cbegin() + found;
    (*iter)->setParent(nullptr);
    _children.erase(iter);

    markAsPending();
}

void SplitPanel::clearChildren() {

    for (const auto& child : _children) {
        child->setParent(nullptr);
    }

    _children.clear();
    _slots.clear();

    markAsPending();
}

math::vec2 SplitPanel::prefetchDesiredSize(cref<ReflowState> state_, float scale_) const {

    math::vec2 childAggregate;
    math::vec2 childMax;

    for (const auto& child : *children()) {

        const auto* const childState = state_.getStateOf(child);

        childAggregate += child->getDesiredSize();
        childMax = math::compMax<float>(childMax, child->getDesiredSize());
    }

    /**/

    const math::vec2 innerSize = attr.orientation.getValue() == ReflowOrientation::eHorizontal ?
                                     math::vec2 { childAggregate.x, childMax.y } :
                                     math::vec2 { childMax.x, childAggregate.y };
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

math::vec2 SplitPanel::computeDesiredSize(cref<ReflowPassState> passState_) const {

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

void SplitPanel::applyLayout(ref<ReflowState> state_, mref<LayoutContext> ctx_) {

    const auto innerSize = layout::outerToInnerSize(attr, ctx_.localSize);
    const auto mainOffset = layout::outerToInnerOffset(attr, ctx_.localOffset);

    for (size_t i = 0; i < _children.size(); ++i) {

        const auto& child = _children[i];
        const auto& slot = _slots[i];

        /**/

        math::vec2 size = innerSize;
        math::vec2 offset = mainOffset;

        if (attr.orientation.getValue() == ReflowOrientation::eHorizontal) {

            size.x = innerSize.x * slot.proportion;
            offset += (size - child->getDesiredSize()) / 2.F;// TODO: We need a correct alignment...
            offset = math::compMax<float>(offset, mainOffset);

        } else {

            size.y = innerSize.y * slot.proportion;
            offset += (size - child->getDesiredSize()) / 2.F;// TODO: We need a correct alignment...
            offset = math::compMax<float>(offset, mainOffset);
        }

        /**/

        const auto childState = state_.getStateOf(child);

        childState->layoutOffset = offset;
        childState->layoutSize = size;
    }

}

void SplitPanel::recomputeSlotPortions(const float unnormalized_) {

    if (_slots.empty()) {
        return;
    }

    /**/

    // Warning: This could introduce some shift as soon as we remove elements with de-normalized proportions
    u32 effective = (0.F - unnormalized_ > std::numeric_limits<float>::epsilon()) ? 1 : 0;
    float aggregate = unnormalized_;

    for (const auto& slot : _slots) {
        if (slot.proportion > 0.F) {
            aggregate += slot.proportion;
            ++effective;
        }
    }

    /**/

    const float prevRel = aggregate / static_cast<float>(effective);
    const float nextRel = 1.F / static_cast<float>(_slots.size());

    /**/

    for (auto& slot : _slots) {

        if (slot.proportion <= 0.F) {
            slot.proportion = nextRel;
            continue;
        }

        const auto prevPropRelDiff = slot.proportion - prevRel;
        const auto nextPropRelDiff = (prevPropRelDiff /* un-scale change proportion */ * effective) /
            static_cast<float>(_slots.size()) /* scale to new set*/;

        slot.proportion = nextRel + nextPropRelDiff;
    }
}
