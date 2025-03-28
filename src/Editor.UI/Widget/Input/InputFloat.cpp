#include "InputFloat.hpp"

#include <format>
#include <Engine.Common/Discard.hpp>
#include <Engine.Reflow/DataWatcher.hpp>
#include <Engine.Reflow/Window/WindowManager.hpp>
#include <Engine.Reflow.Attributes/Layout/Base/BoxLayoutAttributes.hpp>
#include <Engine.Reflow.Uikit/Atom/Paint.hpp>
#include <Engine.Reflow.Uikit/Molecule/Layout/Stack.hpp>

#include "../../EditorUiEventNames.hpp"
#include "../../Event/InputChangeEvent.hpp"

using namespace hg::editor::ui;
using namespace hg::engine::reflow;
using namespace hg;

InputFloat::InputFloat(mref<SharedPtr<uikit::Stack>> content_, mref<SharedPtr<uikit::Text>> text_) :
	Input(hg::move(content_), ReflowClassList {}, nullptr),
	Atom(
		{
			InputFloatDataAttributes {
				math::dvec2 { -std::numeric_limits<f64>::infinity(), std::numeric_limits<f64>::infinity() },
				InputFloatPrecision::eF32,
				0.,
				0.
			}
		},
		{},
		{}
	),
	_text(hg::move(text_)),
	_stringified() {}

InputFloat::~InputFloat() = default;

string InputFloat::getTag() const noexcept {
	return std::format(R"(InputFloat <{:#x}>)", reinterpret_cast<u64>(this));
}

bool InputFloat::shouldTick() const noexcept {
	return not state().isFocus() && isPristine() && state().isLayoutOrRenderPending();
}

void InputFloat::tick() {

	if (isTouched() && isPristine()) {
		markAsUntouched();
	}

	const auto& dataValue = getDataAttributes().valueOf<attr::InputData::value>();
	_stringified = (getDataAttributes().valueOf<attr::InputData::precision>() == InputFloatPrecision::eFloat) ?
		std::format(R"({:.5})", static_cast<f32>(dataValue)) :
		std::format(R"({:.8})", static_cast<f64>(dataValue));

	_text->setText(_stringified);
}

SharedPtr<uikit::Stack> InputFloat::getContent() const noexcept {
	return std::static_pointer_cast<uikit::Stack>(_children.getChild());
}

InputFloat::input_type InputFloat::value() const noexcept {
	return getDataAttributes().valueOf<attr::InputData::value>();
}

void InputFloat::setValue(value_type next_) {
	if (getDataAttributes().update<attr::InputData::value>(next_)) {
		markAsPending(true, true);
	}
}

void InputFloat::updateValueAndValidity(const bool propagate_, const bool emit_) {

	auto next = f64 { 0.F };
	if (not _stringified.empty()) {
		if (getDataAttributes().valueOf<attr::InputData::precision>() == InputFloatPrecision::eF32) {
			// Error: Will throw
			next = std::stof(_stringified);
		} else {
			// Error: Will throw
			next = static_cast<f64>(std::stold(_stringified));
		}
	}

	const auto& limits = getDataAttributes().valueOf<attr::InputData::limits>();
	if (next < limits.min || limits.max < next) {
		_status = InputControlStatus::eInvalid;
		return;
	}

	/**/

	_status = InputControlStatus::eValid;
	setValue(next);
	::hg::discard = _emitter.emit(EditorUiEventNames::InputChangeEvent, InputChangeEvent { shared_from_this() });

	markAsPristine();
}

Widget::listen_handle_type InputFloat::onInputChange(listen_fn_type<InputChangeEvent> listenFn_) {
	return _emitter.on(EditorUiEventNames::InputChangeEvent, hg::move(listenFn_));
}

EventResponse InputFloat::invokeOnKeyDown(ref<const KeyboardEvent> event_) {

	if (not state().isFocus()) {
		return EventResponse::eUnhandled;
	}

	/**/

	if (
		not(event_._key >= 0x30 && event_._key <= 0x39) &&
		not(event_._key >= 0x58 && event_._key <= 0x62 && event_._modifier == 4096) &&
		not(event_._key >= 0x2C && event_._key <= 0x2E) &&
		not(event_._key != '-') &&
		not(event_._key == '\b' || event_._key == '\r' || event_._key == '\x1B')
	) {
		return EventResponse::eConsumed;
	}

	/**/

	const auto& limits = getDataAttributes().valueOf<attr::InputData::limits>();

	/**/

	if (event_._key == 0x2C || event_._key == 0x2E) {

		if (_stringified.find(0x2C) != std::string::npos) {
			return EventResponse::eConsumed;
		}

		if (_stringified.find(0x2E) != std::string::npos) {
			return EventResponse::eConsumed;
		}

		//_value.append(&event_._key, 1);
		constexpr char code { 0x2E };
		_stringified.append(&code, 1);

	} else if (event_._key >= 0x30 && event_._key <= 0x39) {
		const auto maxChars { static_cast<u64>(std::floor(std::log10(static_cast<long double>(limits.max)))) + 1uLL };

		if (_stringified.size() >= maxChars) {
			return EventResponse::eConsumed;
		}

		_stringified.append(&event_._key, 1);

	} else if (event_._key >= 0x58 && event_._key <= 0x62 && event_._modifier == 4096) {

		const auto maxChars { static_cast<u64>(std::floor(std::log10(static_cast<long double>(limits.max)))) + 1uLL };
		char numChar = event_._key < 0x62 ? event_._key - 0x28u : '0';

		if (_stringified.size() >= maxChars) {
			return EventResponse::eConsumed;
		}

		_stringified.append(&numChar, 1);

	} else if (event_._key == '-') {

		if (not _stringified.empty()) {
			return EventResponse::eConsumed;
		}

		_stringified.append(&event_._key, 1);

	} else if (event_._key == /* Backspace */'\b') {

		if (_stringified.empty()) {
			return EventResponse::eConsumed;
		}

		_stringified = _stringified.substr(0, _stringified.size() - 1);

	} else if (event_._key == /* Return */'\r' || event_._key == /* Escape */'\x1B') {

		const auto window { root() };
		const FocusEvent focusEvent { window };
		WindowManager::get()->dispatch<FocusEvent>(std::static_pointer_cast<Window, Widget>(window), focusEvent);

		updateValueAndValidity(true, true);
		return EventResponse::eConsumed;
	}

	_text->setText(_stringified);
	markAsDirty();
	return EventResponse::eConsumed;
}

EventResponse InputFloat::invokeOnBlur(cref<FocusEvent> event_) {
	updateValueAndValidity(true, true);
	return Input::invokeOnBlur(event_);
}

void InputFloat::onAttachDataWatcher(ref<engine::reflow::DataWatcher> watcher_) {
	if (getDataAttributes().queryOf<attr::InputData::value>() != nullptr) {
		watcher_.attach(this, getDataAttributes().get<attr::InputData::value>());
	}
	Input::onAttachDataWatcher(watcher_);
}
void InputFloat::onDetachDataWatcher(ref<engine::reflow::DataWatcher> watcher_) {
	if (getDataAttributes().queryOf<attr::InputData::value>() != nullptr) {
		watcher_.detach(getDataAttributes().get<attr::InputData::value>());
	}
	Input::onDetachDataWatcher(watcher_);
}

/**/

SharedPtr<InputFloat> editor::ui::makeInputFloat() {

	const auto content = make_sptr<uikit::Stack>();
	content->getLayoutAttributes().update<engine::reflow::attr::BoxLayout::placement>(ReflowPlacement::eMiddleLeft);
	content->addChild(make_sptr<uikit::Paint>(ReflowClassList { "[Input] > background"sv }, nullptr));

	auto text = make_sptr<uikit::Text>();
	content->addChild(clone(text));

	auto result = make_sptr<InputFloat>(clone(content), hg::move(text));
	content->setParent(result);

	return result;
}
