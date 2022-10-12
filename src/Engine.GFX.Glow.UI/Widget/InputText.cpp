#include "InputText.hpp"

using namespace ember::engine::gfx::glow::ui;
using namespace ember;

InputText::InputText() :
    Input(),
    _placeholder(),
    _font(nullptr),
    _fontSize(16.F),
    _lineHeight(1.F) {}

cref<string> InputText::placeholder() const noexcept {
    return _placeholder;
}

void InputText::setPlaceholder(cref<string> text_) {
    _placeholder = text_;
}
