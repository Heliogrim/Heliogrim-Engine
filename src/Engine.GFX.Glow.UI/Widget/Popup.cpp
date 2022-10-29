#include "Popup.hpp"

using namespace ember::engine::gfx::glow::ui;
using namespace ember;

Popup::Popup() :
    Widget(),
    _anchor(),
    _root() {
    _reflowPosition = ReflowPosition::eAbsolute;
}

math::vec2 Popup::flow(cref<UIContext> ctx_, cref<math::vec2> available_) {

    math::vec2 zero { 0.F };
    math::vec2 proxy { ctx_.viewport.width, ctx_.viewport.height };

    // Ignore own size and children (report zero) cause we don't care about hierarchical space flow
    [[maybe_unused]] auto selfFlow { Widget::flow(ctx_, proxy) };

    return zero;
}

void Popup::shift(cref<UIContext> ctx_, cref<math::vec2> offset_) {
    // Ignore intrusive layout shift for dismounted component
    Widget::shift(ctx_, _anchor);
}

cref<math::vec2> Popup::anchor() const noexcept {
    return _anchor;
}

void Popup::setAnchor(cref<math::vec2> anchor_) {
    _anchor = anchor_;
}

cref<wptr<Widget>> Popup::root() const noexcept {
    return _root;
}

void Popup::setRoot(cref<wptr<Widget>> root_) {
    _root = root_;
}
