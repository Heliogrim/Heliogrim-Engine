#include "InputText.hpp"

#include <Engine.Common/Make.hpp>

using namespace ember::engine::reflow;
using namespace ember;

InputText::InputText(mref<sptr<BoundStyleSheet>> style_, mref<sptr<BoundStyleSheet>> textStyle_) :
    Input(),
    _wrapper(make_sptr<HBox>(_STD move(style_))),
    _text(make_sptr<Text>(_STD move(textStyle_))) {
    /**/
    _wrapper->addChild(_text);
}

InputText::~InputText() = default;

void InputText::setPlaceholder(cref<string> placeholder_) {
    _placeholder = placeholder_;

    if (_value.empty() && _text) {
        _text->setText(_placeholder);
    }
}

InputText::input_type InputText::value() const noexcept {
    return _value;
}

void InputText::render(const ptr<ReflowCommandBuffer> cmd_) {
    _wrapper->render(cmd_);
    _text->render(cmd_);
}

void InputText::flow(cref<FlowContext> ctx_, cref<math::vec2> space_, ref<StyleKeyStack> styleStack_) {
    _wrapper->setParent(shared_from_this());
    _wrapper->flow(ctx_, space_, styleStack_);
}

void InputText::shift(cref<FlowContext> ctx_, cref<math::vec2> offset_) {
    _wrapper->shift(ctx_, offset_);
}

math::vec2 InputText::outerSize() const noexcept {
    return _wrapper->outerSize();
}

math::vec2 InputText::innerSize() const noexcept {
    return _wrapper->innerSize();
}

math::vec2 InputText::screenOffset() const noexcept {
    return _wrapper->screenOffset();
}

EventResponse InputText::onFocus(cref<FocusEvent> event_) {
    _text->setText(_value);

    _wrapper->state().focus = true;
    return Input<std::string>::onFocus(event_);
}

EventResponse InputText::onBlur(cref<FocusEvent> event_) {

    if (_value.empty()) {
        _text->setText(_placeholder);
    }

    _wrapper->state().focus = false;
    return Input<std::string>::onBlur(event_);
}

EventResponse InputText::onKeyDown(cref<KeyboardEvent> event_) {

    if (not _state.focus) {
        return EventResponse::eUnhandled;
    }

    if (event_._key == /* Backspace */'\b') {
        if (not _value.empty()) {
            _value = _value.substr(0, _value.size() - 1);
        }
        // TODO:

    } else if (event_._key == /* Return */'\r') {
        // TODO:

    } else if (event_._key == /* Escape */'\x1B') {
        // TODO:

    } else if (event_._key < 0x20 || event_._key > 0x7E) {
        return EventResponse::eConsumed;

    } else {

        if (event_._modifier & 0x40 || event_._modifier & 0x80 /* Ctrl + <> */) {
            // TODO:

        } else if (event_._modifier & 0x1 || event_._modifier & 0x2 /* Shift L/R */) {

            char code { event_._key };
            if (code >= 0x61 && code < 0x7A) {
                code -= 32;
            }

            _value.append(&code, 1);

        } else {
            _value.append(&event_._key, 1);
        }

    }

    _text->setText(_value);
    return EventResponse::eConsumed;
}

EventResponse InputText::onKeyUp(cref<KeyboardEvent> event_) {

    if (not _state.focus) {
        return EventResponse::eUnhandled;
    }

    return EventResponse::eConsumed;
}
