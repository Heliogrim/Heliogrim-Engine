#include "Button.hpp"

using namespace ember::engine::gfx::glow::ui;
using namespace ember;

Button::Button() :
    StatefulWidget(),
    _raisedColor(engine::color { 53.F, 57.F, 65.F, 255.F }) {
    _extent.x = -1.F;
    _extent.y = -1.F;

    _color = engine::color { 46.F, 50.F, 58.F, 255.F };
};

u64 Button::addOnClick(mref<OnClickListener> listener_) {
    return _emitter.on<input::event::MouseButtonEvent>(_STD move(listener_));
}

engine::color Button::statedColor() const noexcept {
    if (hovered()) {
        return _raisedColor;
    }
    return StatefulWidget::statedColor();
}

bool Button::onMouseMotionEvent(cref<math::ivec2> pointer_, cref<math::ivec2> delta_, u32 button_, u32 modifier_) {
    // Suppress further forward propagation
    if (contains(pointer_)) {
        return true;
    }
    return false;
}

bool Button::onMouseEnterEvent(cref<math::ivec2> pointer_, bool enter_) {

    for (const auto& node : _nodes) {
        node->onMouseEnterEvent(pointer_, enter_);
    }

    return StatefulWidget::onMouseEnterEvent(pointer_, enter_);
}
