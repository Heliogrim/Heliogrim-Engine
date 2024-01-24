#include "BoxPanel.hpp"

#include <format>
#include "../Layout/Style.hpp"
#include "../Algorithm/Flex.hpp"

using namespace hg::engine::reflow;
using namespace hg;

BoxPanel::BoxPanel() :
    Panel(),
    _attr(
        Attributes {
            .minWidth = { this, { ReflowUnitType::eAuto, 0.F } },
            .width = { this, { ReflowUnitType::eAuto, 0.F } },
            .maxWidth = { this, { ReflowUnitType::eAuto, 0.F } },
            .minHeight = { this, { ReflowUnitType::eAuto, 0.F } },
            .height = { this, { ReflowUnitType::eAuto, 0.F } },
            .maxHeight = { this, { ReflowUnitType::eAuto, 0.F } },
            .padding = { this, Padding { 0.F } },
            .justify = { this, ReflowSpacing::eStart },
            .align = { this, ReflowAlignment::eStart }
        }
    ),
    _children() {}

BoxPanel::~BoxPanel() = default;

string BoxPanel::getTag() const noexcept {
    return std::format(R"(BoxPanel <{:#x}>)", reinterpret_cast<u64>(this));
}

ref<BoxPanel::Attributes> BoxPanel::attributes() noexcept {
    return _attr;
}

const ptr<const Children> BoxPanel::children() const {
    return &_children;
}

void BoxPanel::setChild(cref<sptr<Widget>> nextChild_) {

    _children.getChild()->setParent(nullptr);

    nextChild_->setParent(shared_from_this());
    _children.setChild(nextChild_);

    markAsPending();
}

math::vec2 BoxPanel::prefetchDesiredSize(cref<ReflowState> state_, float scale_) const {

    const auto inner = _children.getChild()->getDesiredSize();
    const auto& attr = const_cast<this_type*>(this)->attributes();
    auto size = layout::innerToOuterSize(attr, inner);

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

math::vec2 BoxPanel::computeDesiredSize(cref<ReflowPassState> passState_) const {

    const auto& attr = const_cast<this_type*>(this)->attributes();

    math::vec2 desired = getDesiredSize();
    if (attr.width->type == ReflowUnitType::eRelative) {
        desired.x = passState_.referenceSize.x * attr.width->value;
    }

    if (attr.height->type == ReflowUnitType::eRelative) {
        desired.y = passState_.referenceSize.y * attr.height->value;
    }

    /**/

    return layout::clampSize(attr, passState_.layoutSize, desired);
}

void BoxPanel::applyLayout(ref<ReflowState> state_, mref<LayoutContext> ctx_) {

    const auto& attr = attributes();

    const auto innerOffset = layout::outerToInnerOffset(attr, ctx_.localOffset);
    const auto innerSize = layout::outerToInnerSize(attr, ctx_.localSize);

    const auto childSize = math::compMin<float>(
        math::compMax<float>(_children.getChild()->getDesiredSize(), math::vec2 { 0.F }),
        innerSize
    );

    /**/

    math::vec2 space = innerSize - childSize;
    space = math::compMax<float>(space, math::vec2 { 0.F });

    switch (attr.justify.getValue()) {
        case ReflowSpacing::eStart:
        case ReflowSpacing::eSpaceBetween: {
            space.x = 0.F;
            break;
        }
        case ReflowSpacing::eSpaceAround:
        case ReflowSpacing::eSpaceEvenly: {
            space.x /= 2.F;
            break;
        }
        case ReflowSpacing::eEnd: {
            break;
        }
    }

    switch (ReflowAlignment::eCenter) {
        case ReflowAlignment::eStart: {
            space.y = 0.F;
            break;
        }
        case ReflowAlignment::eCenter: {
            space.y /= 2.F;
            break;
        }
        case ReflowAlignment::eEnd: {
            break;
        }
    }

    /**/

    auto* const childPassState = state_.getStateOf(_children.getChild());

    childPassState->layoutOffset = innerOffset + space;
    childPassState->layoutSize = childSize;
}
