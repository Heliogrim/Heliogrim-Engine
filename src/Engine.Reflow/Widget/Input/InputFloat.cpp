#include "InputFloat.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Reflow/Window/WindowManager.hpp>

using namespace ember::engine::reflow;
using namespace ember;

InputFloat::InputFloat(mref<sptr<BoundStyleSheet>> style_, mref<sptr<BoundStyleSheet>> textStyle_) :
    Input(),
    _wrapper(make_sptr<HBox>(_STD move(style_))),
    _text(make_sptr<Text>(_STD move(textStyle_))),
    _value(R"(0)"),
    _limits(_STD numeric_limits<input_type>::min(), _STD numeric_limits<input_type>::max()),
    _stepSize(0),
    _precision(InputFloatPrecision::eFloat) {
    /**/
    _wrapper->addChild(_text);
}

InputFloat::~InputFloat() = default;

void InputFloat::valueToText() {

    const auto val { value() };

    string txt {};
    if (_precision == InputFloatPrecision::eFloat) {
        txt = _STD format(R"({:f})", static_cast<float>(val));
    } else {
        txt = _STD format(R"({:f})", static_cast<long double>(val));
    }

    auto it { txt.rbegin() };
    for (; it != txt.rend(); ++it) {
        if ((*it) == '.') {
            ++it;
            break;
        }

        if ((*it) != '0') {
            break;
        }
    }
    txt.erase(it.base(), txt.end());

    _text->setText(txt);
}

void InputFloat::setValue(const input_type value_) {
    _value = _STD format(R"({:})", value_);
    valueToText();
}

void InputFloat::setLimits(const input_type lower_, const input_type upper_) {
    _limits.min = lower_;
    _limits.max = upper_;
}

void InputFloat::setStepSize(const input_type stepSize_) {
    _stepSize = stepSize_;
}

void InputFloat::setPrecision(const InputFloatPrecision precision_) {
    _precision = precision_;
}

void InputFloat::updateValueAndValidity(const bool propagate_, const bool emit_) {

    // TODO: Try to poll last value

    const auto val { value() };

    if (_precision == InputFloatPrecision::eFloat) {
        _value = _STD format(R"({})", static_cast<float>(val));
    } else if (_precision == InputFloatPrecision::eDouble) {
        _value = _STD format(R"({})", static_cast<long double>(val));
    }

    if (val < _limits.min || _limits.max < val) {
        _status = InputControlStatus::eInvalid;
    }

    _status = InputControlStatus::eValid;

    // TODO:
    if (_callback) {
        _callback(value());
    }
}

InputFloat::input_type InputFloat::value() const noexcept {
    if (_value.empty()) {
        return 0.0;
    }

    return _STD stold(_value);
}

void InputFloat::render(const ptr<ReflowCommandBuffer> cmd_) {
    _wrapper->render(cmd_);
}

void InputFloat::flow(cref<FlowContext> ctx_, cref<math::vec2> space_, ref<StyleKeyStack> styleStack_) {
    _wrapper->setParent(shared_from_this());
    _wrapper->flow(ctx_, space_, styleStack_);
}

void InputFloat::shift(cref<FlowContext> ctx_, cref<math::vec2> offset_) {
    _wrapper->shift(ctx_, offset_);
}

math::vec2 InputFloat::outerSize() const noexcept {
    return _wrapper->outerSize();
}

math::vec2 InputFloat::innerSize() const noexcept {
    return _wrapper->innerSize();
}

math::vec2 InputFloat::screenOffset() const noexcept {
    return _wrapper->screenOffset();
}

EventResponse InputFloat::onBlur(cref<FocusEvent> event_) {
    updateValueAndValidity(true, false);
    valueToText();
    return Input<InputFloat::input_type>::onBlur(event_);
}

EventResponse InputFloat::onKeyDown(cref<KeyboardEvent> event_) {

    if (not _state.focus) {
        return EventResponse::eUnhandled;
    }

    if (
        not (event_._key >= 0x30 && event_._key <= 0x39) &&
        not (event_._key >= 0x2C && event_._key <= 0x2E) &&
        not (event_._key != '-') &&
        not (event_._key == '\b' || event_._key == '\r' || event_._key == '\x1B')
    ) {
        return EventResponse::eConsumed;
    }

    if (event_._key == 0x4D) {

        if (not _value.empty()) {
            return EventResponse::eConsumed;
        }

        _value.append(&event_._key, 1);

    } else if (event_._key == 0x2C || event_._key == 0x2E) {

        if (_value.find(0x2C) != _STD string::npos) {
            return EventResponse::eConsumed;
        }

        if (_value.find(0x2E) != _STD string::npos) {
            return EventResponse::eConsumed;
        }

        //_value.append(&event_._key, 1);
        constexpr char code { 0x2E };
        _value.append(&code, 1);

    } else if (event_._key >= 0x30 && event_._key <= 0x39) {

        const auto maxChars { static_cast<u64>(_STD floor(_STD log10(static_cast<long double>(_limits.max)))) + 1ui64 };

        if (_value.size() >= maxChars) {
            return EventResponse::eConsumed;
        }

        _value.append(&event_._key, 1);

    } else if (event_._key == '-') {

        if (not _value.empty()) {
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

    _text->setText(_value);
    return EventResponse::eConsumed;
}

EventResponse InputFloat::onKeyUp(cref<KeyboardEvent> event_) {

    if (not _state.focus) {
        return EventResponse::eUnhandled;
    }

    return EventResponse::eConsumed;
}
