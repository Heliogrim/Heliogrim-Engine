#include "Dialog.hpp"

#include <format>
#include "../Layout/Style.hpp"

using namespace hg::engine::reflow;
using namespace hg;

Dialog::Dialog() :
    Popup(),
    attr(
        Attributes {
            .minWidth = { this, { ReflowUnitType::eAuto, 0.F } },
            .width = { this, { ReflowUnitType::eAuto, 0.F } },
            .maxWidth = { this, { ReflowUnitType::eAuto, 0.F } },
            .minHeight = { this, { ReflowUnitType::eAuto, 0.F } },
            .height = { this, { ReflowUnitType::eAuto, 0.F } },
            .maxHeight = { this, { ReflowUnitType::eAuto, 0.F } },
            .padding = { this, Padding { 0.F } },
            .color = { this, engine::color { 255.F, 255.F, 255.F, 255.F } }
        }
    ) {}

Dialog::~Dialog() = default;

string Dialog::getTag() const noexcept {
    return std::format(R"(Dialog <{:#x}>)", reinterpret_cast<u64>(this));
}

const ptr<const Children> Dialog::children() const {
    return &_children;
}

sptr<Widget> Dialog::getTitleBar() const noexcept {
    return _children.getChild<0>();
}

void Dialog::setTitleBar(cref<sptr<Widget>> titleBar_) {
    titleBar_->setParent(shared_from_this());
    _children.getChild<0>()->setParent(nullptr);
    _children.setChild<0>(titleBar_);
}

sptr<Widget> Dialog::getContent() const noexcept {
    return _children.getChild<1>();
}

void Dialog::setContent(cref<sptr<Widget>> content_) {
    content_->setParent(shared_from_this());
    _children.getChild<1>()->setParent(nullptr);
    _children.setChild<1>(content_);
}

void Dialog::render(const ptr<ReflowCommandBuffer> cmd_) {

    const math::vec2 off = _layoutState.layoutOffset;
    const math::vec2 size = _layoutState.layoutSize;

    const math::vec2 c0 { off.x, off.y };
    const math::vec2 c1 { off.x + size.x, off.y };
    const math::vec2 c2 { off.x + size.x, off.y + size.y };
    const math::vec2 c3 { off.x, off.y + size.y };

    cmd_->drawQuad(c0, c1, c2, c3, attr.color.getValue());

    /**/

    for (const auto& child : _children) {
        child->render(cmd_);
    }
}

math::vec2 Dialog::prefetchDesiredSize(cref<ReflowState> state_, float scale_) const {

    const auto titleBarSize = getTitleBar()->getDesiredSize();
    const auto contentSize = getContent()->getDesiredSize();

    const math::vec2 innerSize { MAX(titleBarSize.x, contentSize.x), titleBarSize.y + contentSize.y };
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

math::vec2 Dialog::computeDesiredSize(cref<ReflowPassState> passState_) const {

    const auto titleBarSize = getTitleBar()->getDesiredSize();
    const auto contentSize = getContent()->getDesiredSize();

    const math::vec2 innerSize { MAX(titleBarSize.x, contentSize.x), titleBarSize.y + contentSize.y };
    math::vec2 size = layout::innerToOuterSize(attr, innerSize);

    /**/

    if (attr.width->type == ReflowUnitType::eAbsolute) {
        size.x = attr.width->value;
    } else if (attr.width->type == ReflowUnitType::eRelative) {
        size.x = passState_.referenceSize.x * attr.width->value;
    }

    if (attr.height->type == ReflowUnitType::eAbsolute) {
        size.y = attr.height->value;
    } else if (attr.height->type == ReflowUnitType::eRelative) {
        size.y = passState_.referenceSize.y * attr.height->value;
    }

    /**/

    return layout::clampSize(attr, passState_.layoutSize, size);

}

void Dialog::applyLayout(ref<ReflowState> state_, mref<LayoutContext> ctx_) {

    const auto innerOffset = layout::outerToInnerOffset(attr, ctx_.localOffset);
    const auto innerSize = layout::outerToInnerSize(attr, ctx_.localSize);

    /**/

    const auto titleBar = getTitleBar();
    auto* const titleState = state_.getStateOf(titleBar);

    const auto titleSize = math::compMin<float>(titleBar->getDesiredSize(), innerSize);

    titleState->layoutOffset = innerOffset;
    titleState->layoutSize = titleSize;

    /**/

    const auto content = getContent();
    auto* const contentState = state_.getStateOf(getContent());

    const auto leftSpace = math::vec2 { innerSize.x, innerSize.y - titleSize.y };
    const auto contentSize = math::compMin<float>(content->getDesiredSize(), leftSpace);

    contentState->layoutOffset = innerOffset + math::vec2 { 0.F, titleSize.y };
    contentState->layoutSize = contentSize;
}
