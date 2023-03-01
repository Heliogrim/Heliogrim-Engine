#include "Overlay.hpp"

using namespace hg::engine::reflow;
using namespace hg;

Overlay::Overlay() :
    Widget() {}

Overlay::~Overlay() = default;

string Overlay::getTag() const noexcept {
    return _STD format(R"(Overlay <{:#x}>)", reinterpret_cast<u64>(this));
}

sptr<Widget> Overlay::getContent() const noexcept {
    return _content;
}

void Overlay::setContent(cref<sptr<Widget>> content_) {

    content_->setParent(shared_from_this());

    if (_content && _content->parent().get() == this) {
        _content->setParent(nullptr);
    }

    _content = content_;
}

void Overlay::render(const ptr<ReflowCommandBuffer> cmd_) {

    if (!_content) {
        return;
    }

    _content->render(cmd_);
}

void Overlay::flow(
    cref<FlowContext> ctx_,
    cref<math::vec2> space_,
    cref<math::vec2> limit_,
    ref<StyleKeyStack> styleStack_
) {

    if (!_content) {
        return;
    }

    _content->flow(ctx_, space_, limit_, styleStack_);
}

void Overlay::shift(cref<FlowContext> ctx_, cref<math::vec2> offset_) {

    if (not _content) {
        return;
    }

    _content->shift(ctx_, screenOffset());
}

const ptr<const Children> Overlay::children() const {
    return nullptr;
}

ReflowPosition Overlay::position() const noexcept {
    return ReflowPosition::eAbsolute;
}

math::vec2 Overlay::outerSize() const noexcept {
    if (_content) {
        return _content->outerSize();
    }
    return math::vec2 { 0.F };
}

math::vec2 Overlay::innerSize() const noexcept {
    if (_content) {
        return _content->innerSize();
    }
    return math::vec2 { 0.F };
}

math::vec2 Overlay::screenOffset() const noexcept {

    if (hasParent()) {
        return parent()->screenOffset();
    }

    return math::vec2 { 0.F };
}
