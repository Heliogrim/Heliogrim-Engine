#include "Input.hpp"

using namespace ember::engine::reflow;
using namespace ember;

bool InputBase::isEnabled() const noexcept {
    return _status != InputControlStatus::eDisabled;
}

bool InputBase::isDirty() const noexcept {
    return _dirty;
}

bool InputBase::isTouched() const noexcept {
    return _touched;
}

void InputBase::enable() {
    if (_status != InputControlStatus::eDisabled) {
        return;
    }

    // TODO: We need to dispatch the validation to get the actual status
    _status = InputControlStatus::ePending;
    updateValueAndValidity(false, false);
}

void InputBase::disable() {
    _status = InputControlStatus::eDisabled;
}

void InputBase::markAsDirty() {
    _dirty = true;
}

void InputBase::markAsPristine() {
    _dirty = false;
}

void InputBase::markAsTouched() {
    _touched = true;
}

void InputBase::markAsUntouched() {
    _touched = false;
}

void InputBase::reset() {
    //__nop();
}

void InputBase::updateValueAndValidity(const bool propagate_, const bool emit_) {
    _status = InputControlStatus::eValid;

    // TODO: Propagation and Emitting
}

EventResponse InputBase::onFocus(cref<FocusEvent> event_) {
    _state |= WidgetStateFlagBits::eFocus;
    return Widget::onFocus(event_);
}

EventResponse InputBase::onBlur(cref<FocusEvent> event_) {
    markAsTouched();
    _state.unwrap &= (~static_cast<u8>(WidgetStateFlagBits::eFocus));
    return Widget::onBlur(event_);
}
