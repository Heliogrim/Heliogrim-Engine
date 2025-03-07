#include "Input.hpp"

using namespace hg::engine::reflow;
using namespace hg;

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

EventResponse InputBase::invokeOnFocus(cref<FocusEvent> event_) {
    _state |= WidgetStateFlagBits::eFocus;

    // Warning: Temporary Fix
    markAsPending();

    return Widget::invokeOnFocus(event_);
}

EventResponse InputBase::invokeOnBlur(cref<FocusEvent> event_) {
    markAsTouched();
    _state.unwrap &= (~static_cast<u8>(WidgetStateFlagBits::eFocus));

    // Warning: Temporary Fix
    markAsPending();

    return Widget::invokeOnBlur(event_);
}
