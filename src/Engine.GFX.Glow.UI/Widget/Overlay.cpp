#include "Overlay.hpp"

using namespace ember::engine::gfx::glow::ui;
using namespace ember;

Overlay::Overlay() :
    Widget() {
    _reflowPosition = ReflowPosition::eAbsolute;
    _color = engine::color { 0.F, 0.F, 0.F, 0.2F };
}

void Overlay::render(const ptr<UICommandBuffer> cmd_) {
    Reflow::renderContainer(this, cmd_);
}

math::vec2 Overlay::flow(cref<UIContext> ctx_, cref<math::vec2> available_) {

    const math::vec2 zero { 0.F };

    // Ignore own size and children (report zero) cause we don't care about external hierarchical space flow
    [[maybe_unused]] auto selfFlow { Widget::flow(ctx_, available_) };

    return zero;
}

void Overlay::shift(cref<UIContext> ctx_, cref<math::vec2> offset_) {
    // Ignore intrusive layout shift for dismounted component
    const auto* parent { _parent };
    Widget::shift(ctx_, math::vec2 { parent->transform().offsetX, parent->transform().offsetY });
}
