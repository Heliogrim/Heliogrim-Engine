#include "PopupLayer.hpp"

#include "Window.hpp"
#include "Engine.Reflow/Widget/Popup.hpp"

using namespace ember::engine::reflow;
using namespace ember;

PopupLayer::PopupLayer(cref<sptr<Window>> window_, cref<sptr<Popup>> popup_) :
    _window(window_),
    _screenPos(),
    _screenSize(),
    _content(popup_) {
    popup_->setPopupLayer(this);
}

PopupLayer::~PopupLayer() {

    if (_content) {
        _content->setPopupLayer(nullptr);
        _content.reset();
    }
}

sptr<Window> PopupLayer::getWindow() const noexcept {
    return _window;
}

void PopupLayer::setScreenPos(cref<math::vec2> screenPos_) {
    _screenPos = screenPos_;
}

cref<math::vec2> PopupLayer::getScreenPos() const noexcept {
    return _screenPos;
}

void PopupLayer::overrideScreenSize(cref<math::vec2> screenSize_) {
    _screenSize = screenSize_;
}

bool PopupLayer::hasOverrideSize() const noexcept {
    return (_screenSize.x > 0.F) && (_screenSize.y > 0.F);
}

cref<math::vec2> PopupLayer::getOverrideSize() const noexcept {
    return _screenSize;
}

sptr<Popup> PopupLayer::getContent() const noexcept {
    return _content;
}
