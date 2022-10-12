#include "MenuButton.hpp"

#include <Engine.Input/MouseButtonEvent.hpp>

using namespace ember::engine::gfx::glow::ui;
using namespace ember;

MenuButton::MenuButton() :
    Button(),
    _menu(),
    _rootListenerId(~0ui64) {}

bool MenuButton::onMouseButtonEvent(cref<math::ivec2> pointer_, u32 button_, bool down_, u32 modifier_) {

    Widget::onMouseButtonEvent(pointer_, button_, down_, modifier_);

    if (not down_) {
        return false;
    } 

    ptr<Widget> root { _parent };
    while (root->parent()) {
        root = root->parent();
    }
    assert(root);

    if (_rootListenerId != ~0ui64) {
        root->emitter().remove<input::event::MouseButtonEvent>(this->_rootListenerId);
        _rootListenerId = ~0ui64;
    }

    if (!_menu) {
        return false;
    }

    _menu->setAnchor({ _transform.offsetX/* + _transform.width */, _transform.offsetY + _transform.height });
    _menu->setVisible(true);

    _rootListenerId = root->emitter().on<input::event::MouseButtonEvent>(
        [this, root = root](cref<input::event::MouseButtonEvent> event_) {

            if (not event_._down || this->_menu->contains(event_._pointer)) {
                return;
            }

            this->_menu->setVisible(false);
        }
    );

    return true;
}

cref<sptr<Menu>> MenuButton::menu() const noexcept {
    return _menu;
}

void MenuButton::setMenu(cref<sptr<Menu>> menu_) {
    _menu = menu_;
}
