#include "Overlay.hpp"

#include "NullWidget.hpp"

using namespace hg::engine::reflow;
using namespace hg;

Overlay::Overlay() :
    Widget(),
    _children() {
    _children.setChild(NullWidget::instance());
}

Overlay::~Overlay() = default;

string Overlay::getTag() const noexcept {
    return _STD format(R"(Overlay <{:#x}>)", reinterpret_cast<u64>(this));
}

const ptr<const Children> Overlay::children() const {
    return &_children;
}

sptr<Widget> Overlay::getContent() const noexcept {
    return _children.getChild();
}

void Overlay::setContent(cref<sptr<Widget>> content_) {
    content_->setParent(shared_from_this());
    _children.getChild()->setParent(nullptr);
    _children.setChild(content_);
}

void Overlay::render(cref<ReflowState> state_, const ptr<ReflowCommandBuffer> cmd_) {
    _children.getChild()->render(state_, cmd_);
}

ReflowPosition Overlay::position() const noexcept {
    return ReflowPosition::eAbsolute;
}

math::vec2 Overlay::prefetchDesiredSize(cref<ReflowState> state_, float scale_) const {
    return _children.getChild()->getDesiredSize();
}

math::vec2 Overlay::computeDesiredSize(cref<ReflowPassState> passState_) const {
    return _children.getChild()->getDesiredSize();
}

void Overlay::applyLayout(ref<ReflowState> state_, mref<LayoutContext> ctx_) {
    auto* const childState = state_.getStateOf(_children.getChild());
    childState->layoutOffset = ctx_.localOffset;
    childState->layoutSize = ctx_.localSize;
}
