#include "InputIntegral.hpp"

#include <format>
#include <Engine.Common/Make.hpp>
#include <Engine.Reflow/Window/WindowManager.hpp>

using namespace hg::engine::reflow;
using namespace hg;

InputIntegral::InputIntegral() :
	Input(),
	_wrapper(make_sptr<uikit::HorizontalLayout>()),
	_text(make_sptr<uikit::Text>()),
	_value(R"(0)"),
	_limits(std::numeric_limits<s64>::min(), std::numeric_limits<s64>::max()),
	_stepSize(1LL),
	_children() {
	/**/
	_wrapper->addChild(_text);
	_children.setChild(_wrapper);
}

InputIntegral::~InputIntegral() = default;

string InputIntegral::getTag() const noexcept {
	return std::format(R"(InputIntegral <{:#x}>)", reinterpret_cast<u64>(this));
}

void InputIntegral::setValue(const s64 value_) {
	_value = std::to_string(value_);
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

	return std::stoll(_value);
}

const ptr<const SingleChildren> InputIntegral::children() const {
	return &_children;
}

void InputIntegral::render(const ptr<ReflowCommandBuffer> cmd_) {
	_wrapper->render(cmd_);
	_text->render(cmd_);
}

PrefetchSizing InputIntegral::prefetchSizing(ReflowAxis axis_, ref<const ReflowState> state_) const {
	_wrapper->setParent(const_cast<this_type*>(this)->shared_from_this());
	return {
		_children.getChild()->getLayoutState().prefetchMinSize,
		_children.getChild()->getLayoutState().prefetchSize
	};
}

PassPrefetchSizing InputIntegral::passPrefetchSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) const {
	return _children.getChild()->passPrefetchSizing(axis_, passState_);
}

void InputIntegral::computeSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) {
	_children.getChild()->getLayoutState().computeSize = passState_.computeSize;
}

void InputIntegral::applyLayout(ref<ReflowState> state_) {
	const auto childState = state_.getStateOf(_children.getChild());
	childState->layoutOffset = getLayoutState().layoutOffset;
	childState->layoutSize = getLayoutState().layoutSize;
}

math::fvec2 InputIntegral::getShrinkFactor() const noexcept {
	return _children.getChild()->getShrinkFactor();
}

math::fvec2 InputIntegral::getGrowFactor() const noexcept {
	return _children.getChild()->getGrowFactor();
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
		not(event_._key >= 0x30 && event_._key <= 0x39) &&
		not(event_._key >= 0x2C && event_._key <= 0x2E) &&
		not(event_._key == '\b' || event_._key == '\r' || event_._key == '\x1B')
	) {
		return EventResponse::eConsumed;
	}

	if (event_._key == 0x2D) {

		if (not _value.empty()) {
			return EventResponse::eConsumed;
		}

		_value.append(&event_._key, 1);

	} else if (event_._key >= 0x30 && event_._key <= 0x39) {

		const auto maxChars { static_cast<u64>(std::floor(std::log10(static_cast<long double>(_limits.max)))) + 1uLL };

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

		WindowManager::get()->dispatch<FocusEvent>(std::static_pointer_cast<Window, Widget>(window), focusEvent);
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
