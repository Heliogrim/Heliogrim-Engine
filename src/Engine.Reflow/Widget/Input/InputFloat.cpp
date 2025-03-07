#include "InputFloat.hpp"

#include <format>
#include <Engine.Common/Make.hpp>
#include <Engine.Reflow/Window/WindowManager.hpp>

using namespace hg::engine::reflow;
using namespace hg;

InputFloat::InputFloat() :
	Input(),
	_wrapper(make_sptr<uikit::HorizontalLayout>()),
	_text(make_sptr<uikit::Text>()),
	_value(R"(0)"),
	_limits(std::numeric_limits<input_type>::min(), std::numeric_limits<input_type>::max()),
	_stepSize(0),
	_precision(InputFloatPrecision::eFloat),
	_children() {
	/**/
	_wrapper->addChild(_text);
	_children.setChild(_wrapper);
}

InputFloat::~InputFloat() = default;

string InputFloat::getTag() const noexcept {
	return std::format(R"(InputFloat <{:#x}>)", reinterpret_cast<u64>(this));
}

void InputFloat::valueToText() {

	const auto val { value() };

	string txt {};
	if (_precision == InputFloatPrecision::eFloat) {
		txt = std::format(R"({:f})", static_cast<float>(val));
	} else {
		txt = std::format(R"({:f})", static_cast<long double>(val));
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
	_value = std::format(R"({:})", value_);
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
		_value = std::format(R"({})", static_cast<float>(val));
	} else if (_precision == InputFloatPrecision::eDouble) {
		_value = std::format(R"({})", static_cast<long double>(val));
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

	return std::stold(_value);
}

const ptr<const SingleChildren> InputFloat::children() const {
	return &_children;
}

void InputFloat::render(const ptr<ReflowCommandBuffer> cmd_) {
	_wrapper->render(cmd_);
}

PrefetchSizing InputFloat::prefetchSizing(ReflowAxis axis_, ref<const ReflowState> state_) const {
	_wrapper->setParent(const_cast<this_type*>(this)->shared_from_this());
	return {
		_children.getChild()->getLayoutState().prefetchMinSize,
		_children.getChild()->getLayoutState().prefetchSize
	};
}

PassPrefetchSizing InputFloat::passPrefetchSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) const {
	return _children.getChild()->passPrefetchSizing(axis_, passState_);
}

void InputFloat::computeSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) {
	_children.getChild()->getLayoutState().computeSize = passState_.computeSize;
}

void InputFloat::applyLayout(ref<ReflowState> state_) {
	const auto childState = state_.getStateOf(_children.getChild());
	childState->layoutOffset = getLayoutState().layoutOffset;
	childState->layoutSize = getLayoutState().layoutSize;
}

math::fvec2 InputFloat::getShrinkFactor() const noexcept {
	return _children.getChild()->getShrinkFactor();
}

math::fvec2 InputFloat::getGrowFactor() const noexcept {
	return _children.getChild()->getGrowFactor();
}

EventResponse InputFloat::invokeOnFocus(cref<FocusEvent> event_) {
	_wrapper->state() |= WidgetStateFlagBits::eFocus;
	return Input<InputFloat::input_type>::invokeOnFocus(event_);
}

EventResponse InputFloat::invokeOnBlur(cref<FocusEvent> event_) {
	updateValueAndValidity(true, false);
	valueToText();

	_wrapper->state().unwrap &= (~static_cast<WidgetState::value_type>(WidgetStateFlagBits::eFocus));
	return Input<InputFloat::input_type>::invokeOnBlur(event_);
}

EventResponse InputFloat::invokeOnKeyDown(ref<const KeyboardEvent> event_) {

	if (not _state.isFocus()) {
		return EventResponse::eUnhandled;
	}

	if (
		not(event_._key >= 0x30 && event_._key <= 0x39) &&
		not(event_._key >= 0x2C && event_._key <= 0x2E) &&
		not(event_._key != '-') &&
		not(event_._key == '\b' || event_._key == '\r' || event_._key == '\x1B')
	) {
		return EventResponse::eConsumed;
	}

	if (event_._key == 0x4D) {

		if (not _value.empty()) {
			return EventResponse::eConsumed;
		}

		_value.append(&event_._key, 1);

	} else if (event_._key == 0x2C || event_._key == 0x2E) {

		if (_value.find(0x2C) != std::string::npos) {
			return EventResponse::eConsumed;
		}

		if (_value.find(0x2E) != std::string::npos) {
			return EventResponse::eConsumed;
		}

		//_value.append(&event_._key, 1);
		constexpr char code { 0x2E };
		_value.append(&code, 1);

	} else if (event_._key >= 0x30 && event_._key <= 0x39) {

		const auto maxChars { static_cast<u64>(std::floor(std::log10(static_cast<long double>(_limits.max)))) + 1uLL };

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

		WindowManager::get()->dispatch<FocusEvent>(std::static_pointer_cast<Window, Widget>(window), focusEvent);
		return EventResponse::eConsumed;
	}

	_text->setText(_value);
	return EventResponse::eConsumed;
}

EventResponse InputFloat::invokeOnKeyUp(ref<const KeyboardEvent> event_) {

	if (not _state.isFocus()) {
		return EventResponse::eUnhandled;
	}

	return EventResponse::eConsumed;
}
