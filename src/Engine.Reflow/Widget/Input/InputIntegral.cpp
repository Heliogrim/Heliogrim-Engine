#include "InputIntegral.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Reflow/Window/WindowManager.hpp>

using namespace hg::engine::reflow;
using namespace hg;

InputIntegral::InputIntegral(mref<sptr<BoundStyleSheet>> style_, mref<sptr<BoundStyleSheet>> textStyle_) :
    Input(),
    _wrapper(make_sptr<HBox>(_STD move(style_))),
    _text(make_sptr<Text>(_STD move(textStyle_))),
    _value(R"(0)"),
    _limits(_STD numeric_limits<s64>::min(), _STD numeric_limits<s64>::max()),
    _stepSize(1i64) {
    /**/
    _wrapper->addChild(_text);
}

InputIntegral::~InputIntegral() = default;

string InputIntegral::getTag() const noexcept {
    return _STD format(R"(InputIntegral <{:#x}>)", reinterpret_cast<u64>(this));
}

void InputIntegral::setValue(const s64 value_) {
    _value = _STD to_string(value_);
    _text->setText(_value);
}

void InputIntegral::setLimits(const s64 lower_, const s64 upper_) {
    _limits.min = lower_;
    _limits.max = upper_;
}

void InputIntegral::setStepSize(const s64 stepSize_) {
    _stepSize = stepSize_;
}

void InputIntegral::updateValueAndValidity(const bool propagate_, const bool emit_) {

    // TODO: Try to poll last value

    const auto val { value() };
    if (val < _limits.min || _limits.max < val) {
        _status = InputControlStatus::eInvalid;
    }

    _status = InputControlStatus::eValid;

    // TODO:
    if (_callback) {
        _callback(value());
    }
}

Input<s64>::input_type InputIntegral::value() const noexcept {
    if (_value.empty()) {
        return 0;
    }

    return _STD stoll(_value);
}

void InputIntegral::render(const ptr<ReflowCommandBuffer> cmd_) {
    _wrapper->render(cmd_);
    _text->render(cmd_);
}

void InputIntegral::flow(
    cref<FlowContext> ctx_,
    cref<math::vec2> space_,
    cref<math::vec2> limit_,
    ref<StyleKeyStack> styleStack_
) {
    _wrapper->setParent(shared_from_this());
    _wrapper->flow(ctx_, space_, limit_, styleStack_);
}

void InputIntegral::shift(cref<FlowContext> ctx_, cref<math::vec2> offset_) {
    _wrapper->shift(ctx_, offset_);
}

math::vec2 InputIntegral::outerSize() const noexcept {
    return _wrapper->outerSize();
}

math::vec2 InputIntegral::innerSize() const noexcept {
    return _wrapper->innerSize();
}

math::vec2 InputIntegral::screenOffset() const noexcept {
    return _wrapper->screenOffset();
}

float InputIntegral::shrinkFactor() const noexcept {
    return _wrapper->shrinkFactor();
}

float InputIntegral::growFactor() const noexcept {
    return _wrapper->growFactor();
}

EventResponse InputIntegral::onFocus(cref<FocusEvent> event_) {
    _wrapper->state() |= WidgetStateFlagBits::eFocus;
    return Input<long long>::onFocus(event_);
}

EventResponse InputIntegral::onBlur(cref<FocusEvent> event_) {
    _wrapper->state().unwrap &= (~static_cast<WidgetState::value_type>(WidgetStateFlagBits::eFocus));
    return Input<long long>::onBlur(event_);
}

EventResponse InputIntegral::onKeyDown(cref<KeyboardEvent> event_) {

    if (not _state.isFocus()) {
        return EventResponse::eUnhandled;
    }

    if (
        not (event_._key >= 0x30 && event_._key <= 0x39) &&
        not (event_._key >= 0x2C && event_._key <= 0x2E) &&
        not (event_._key == '\b' || event_._key == '\r' || event_._key == '\x1B')
    ) {
        return EventResponse::eConsumed;
    }

    if (event_._key == 0x2D) {

        if (not _value.empty()) {
            return EventResponse::eConsumed;
        }

        _value.append(&event_._key, 1);

    } else if (event_._key >= 0x30 && event_._key <= 0x39) {

        const auto maxChars { static_cast<u64>(_STD floor(_STD log10(static_cast<long double>(_limits.max)))) + 1ui64 };

        if (_value.size() >= maxChars) {
            return EventResponse::eConsumed;
        }

        _value.append(&event_._key, 1);

    } else if (event_._key == /* Backspace */'\b') {

        if (_value.empty()) {
            return EventResponse::eConsumed;
        }

        _value = _value.substr(0, _value.size() - 1);

    } else if (event_._key == /* Return */'\r' || event_._key == /* Escape */'\x1B') {

        const auto window { root() };
        const FocusEvent focusEvent { window };

        WindowManager::get()->dispatch<FocusEvent>(_STD static_pointer_cast<Window, Widget>(window), focusEvent);
        return EventResponse::eConsumed;
    }

    updateValueAndValidity(true, true);
    _text->setText(_value);

    return EventResponse::eConsumed;
}

EventResponse InputIntegral::onKeyUp(cref<KeyboardEvent> event_) {

    if (not _state.isFocus()) {
        return EventResponse::eUnhandled;
    }

    return EventResponse::eConsumed;
}
