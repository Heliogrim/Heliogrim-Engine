#include "InputText.hpp"

#include <format>
#include <Engine.Common/Make.hpp>
#include <Engine.Reflow/Window/WindowManager.hpp>

using namespace hg::engine::reflow;
using namespace hg;

InputText::InputText() :
	Input(),
	_wrapper(make_sptr<uikit::HorizontalLayout>()),
	_text(make_sptr<uikit::Text>()),
	_children() {
	/**/
	_wrapper->addChild(_text);
	_children.setChild(_wrapper);
}

InputText::~InputText() = default;

string InputText::getTag() const noexcept {
	return std::format(R"(InputText <{:#x}>)", reinterpret_cast<u64>(this));
}

void InputText::setPlaceholder(cref<string> placeholder_) {
	_placeholder = placeholder_;

	if (_value.empty() && _text) {
		_text->setText(_placeholder);
	}
}

InputText::input_type InputText::value() const noexcept {
	return _value;
}

const ptr<const SingleChildren> InputText::children() const {
	return &_children;
}

void InputText::render(const ptr<ReflowCommandBuffer> cmd_) {
	_wrapper->render(cmd_);
	_text->render(cmd_);
}

PrefetchSizing InputText::prefetchSizing(ReflowAxis axis_, ref<const ReflowState> state_) const {
	_wrapper->setParent(const_cast<this_type*>(this)->shared_from_this());
	return {
		_children.getChild()->getLayoutState().prefetchMinSize,
		_children.getChild()->getLayoutState().prefetchSize
	};
}

PassPrefetchSizing InputText::passPrefetchSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) const {
	return _children.getChild()->passPrefetchSizing(axis_, passState_);
}

void InputText::computeSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) {
	_children.getChild()->getLayoutState().computeSize = passState_.computeSize;
}

void InputText::applyLayout(ref<ReflowState> state_) {
	const auto childState = state_.getStateOf(_children.getChild());
	childState->layoutOffset = getLayoutState().layoutOffset;
	childState->layoutSize = getLayoutState().layoutSize;
}

math::fvec2 InputText::getGrowFactor() const noexcept {
	return _children.getChild()->getGrowFactor();
}

math::fvec2 InputText::getShrinkFactor() const noexcept {
	return _children.getChild()->getShrinkFactor();
}

EventResponse InputText::onFocus(cref<FocusEvent> event_) {
    _text->setText(_value);

    _wrapper->state() |= WidgetStateFlagBits::eFocus;
    return Input<std::string>::onFocus(event_);
}

EventResponse InputText::onBlur(cref<FocusEvent> event_) {

	if (_value.empty()) {
		_text->setText(_placeholder);
		markAsPending();
	}

    _wrapper->state().unwrap &= (~static_cast<WidgetState::value_type>(WidgetStateFlagBits::eFocus));
    return Input<std::string>::onBlur(event_);
}

EventResponse InputText::onKeyDown(cref<KeyboardEvent> event_) {

	if (not _state.isFocus()) {
		return EventResponse::eUnhandled;
	}

	if (event_._key == /* Backspace */'\b') {
		if (not _value.empty()) {
			_value = _value.substr(0, _value.size() - 1);
		}
		// TODO:

	} else if (event_._key == /* Return */'\r' || event_._key == /* Escape */'\x1B') {

		const auto window { root() };
		const FocusEvent focusEvent { window };

		WindowManager::get()->dispatch<FocusEvent>(std::static_pointer_cast<Window, Widget>(window), focusEvent);
		return EventResponse::eConsumed;

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

	if (not _state.isFocus()) {
		return EventResponse::eUnhandled;
	}

	return EventResponse::eConsumed;
}
