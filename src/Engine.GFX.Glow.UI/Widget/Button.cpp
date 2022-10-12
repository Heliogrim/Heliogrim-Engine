#include "Button.hpp"

using namespace ember::engine::gfx::glow::ui;
using namespace ember;

Button::Button() :
    StatefulWidget() {
    _extent.x = -1.F;
    _extent.y = -1.F;
};

u64 Button::addOnClick(mref<OnClickListener> listener_) {
    return _emitter.on<input::event::MouseButtonEvent>(_STD move(listener_));
}

bool Button::onMouseEnterEvent(cref<math::ivec2> pointer_, bool enter_) {

    for (const auto& node : _nodes) {
        node->onMouseEnterEvent(pointer_, enter_);
    }

    return StatefulWidget::onMouseEnterEvent(pointer_, enter_);
}
