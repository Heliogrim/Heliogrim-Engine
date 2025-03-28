#include "InputIntegral.hpp"

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

InputIntegral::InputIntegral(mref<SharedPtr<uikit::Stack>> content_, mref<SharedPtr<uikit::Text>> text_) :
	Input(hg::move(content_), ReflowClassList {}, nullptr),
	Atom(
		{
			InputIntegralDataAttributes {
				math::vec2_t<s64> { std::numeric_limits<s64>::min(), std::numeric_limits<s64>::max() },
				InputIntegralPrecision::eS64,
				0.,
				0.
			}
		},
		{},
		{}
	),
	_text(hg::move(text_)),
	_stringified() {}

InputIntegral::~InputIntegral() = default;

string InputIntegral::getTag() const noexcept {
	return std::format(R"(InputIntegral <{:#x}>)", reinterpret_cast<u64>(this));
}

bool InputIntegral::shouldTick() const noexcept {
	return not state().isFocus() && isPristine() && state().isLayoutOrRenderPending();
}

void InputIntegral::tick() {

	if (isTouched() && isPristine()) {
		markAsUntouched();
	}

	const auto& dataValue = getDataAttributes().valueOf<attr::InputData::value>();
	_stringified = std::format(R"({})", static_cast<s64>(dataValue));

	_text->setText(_stringified);
}

SharedPtr<uikit::Stack> InputIntegral::getContent() const noexcept {
	return std::static_pointer_cast<uikit::Stack>(_children.getChild());
}

InputIntegral::input_type InputIntegral::value() const noexcept {
	return getDataAttributes().valueOf<attr::InputData::value>();
}

void InputIntegral::setValue(value_type next_) {
	if (getDataAttributes().update<attr::InputData::value>(next_)) {
		markAsPending(true, true);
	}
}

void InputIntegral::updateValueAndValidity(const bool propagate_, const bool emit_) {

	auto next = s64 { 0LL };
	if (not _stringified.empty()) {
		next = std::stoll(_stringified);
	}

	const auto& limits = getDataAttributes().valueOf<attr::InputData::limits>();
	if (next < limits.min || limits.max < next) {
		_status = InputControlStatus::eInvalid;
		return;
	}

	switch (getDataAttributes().valueOf<attr::InputData::precision>()) {
		case InputIntegralPrecision::eS8: {
			if (next < std::numeric_limits<s8>::min() || std::numeric_limits<s8>::max() < next) {
				_status = InputControlStatus::eInvalid;
				return;
			}
			break;
		}
		case InputIntegralPrecision::eS16: {
			if (next < std::numeric_limits<s16>::min() || std::numeric_limits<s16>::max() < next) {
				_status = InputControlStatus::eInvalid;
				return;
			}
			break;
		}
		case InputIntegralPrecision::eS32: {
			if (next < std::numeric_limits<s32>::min() || std::numeric_limits<s32>::max() < next) {
				_status = InputControlStatus::eInvalid;
				return;
			}
			break;
		}
		case InputIntegralPrecision::eS64: {
			if (next < std::numeric_limits<s64>::min() || std::numeric_limits<s64>::max() < next) {
				_status = InputControlStatus::eInvalid;
				return;
			}
			break;
		}
		case InputIntegralPrecision::eU8: {
			if (next < std::numeric_limits<u8>::min() || std::numeric_limits<u8>::max() < next) {
				_status = InputControlStatus::eInvalid;
				return;
			}
			break;
		}
		case InputIntegralPrecision::eU16: {
			if (next < std::numeric_limits<u16>::min() || std::numeric_limits<u16>::max() < next) {
				_status = InputControlStatus::eInvalid;
				return;
			}
			break;
		}
		case InputIntegralPrecision::eU32: {
			if (next < std::numeric_limits<u32>::min() || std::numeric_limits<u32>::max() < next) {
				_status = InputControlStatus::eInvalid;
				return;
			}
			break;
		}
		case InputIntegralPrecision::eU64: {
			if (next < std::numeric_limits<u64>::min() || std::numeric_limits<u64>::max() < next) {
				_status = InputControlStatus::eInvalid;
				return;
			}
			break;
		}
		default: std::unreachable();
	}

	/**/

	_status = InputControlStatus::eValid;
	setValue(next);
	::hg::discard = _emitter.emit(EditorUiEventNames::InputChangeEvent, InputChangeEvent { shared_from_this() });

	markAsPristine();
}

Widget::listen_handle_type InputIntegral::onInputChange(listen_fn_type<InputChangeEvent> listenFn_) {
	return _emitter.on(EditorUiEventNames::InputChangeEvent, hg::move(listenFn_));
}

EventResponse InputIntegral::invokeOnKeyDown(ref<const KeyboardEvent> event_) {

	if (not state().isFocus()) {
		return EventResponse::eUnhandled;
	}

	/**/

	if (
		not(event_._key >= 0x30 && event_._key <= 0x39) &&
		not(event_._key >= 0x58 && event_._key <= 0x62 && event_._modifier == 4096) &&
		not(event_._key != '-') &&
		not(event_._key == '\b' || event_._key == '\r' || event_._key == '\x1B')
	) {
		return EventResponse::eConsumed;
	}

	/**/

	const auto& limits = getDataAttributes().valueOf<attr::InputData::limits>();

	/**/

	if (event_._key >= 0x30 && event_._key <= 0x39) {
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

		const auto precision = getDataAttributes().valueOf<attr::InputData::precision>();
		const auto allowSigned = precision == InputIntegralPrecision::eS8 ||
			precision == InputIntegralPrecision::eS16 ||
			precision == InputIntegralPrecision::eS32 ||
			precision == InputIntegralPrecision::eS64;
		if (not allowSigned || not _stringified.empty()) {
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

EventResponse InputIntegral::invokeOnBlur(cref<FocusEvent> event_) {
	updateValueAndValidity(true, true);
	return Input::invokeOnBlur(event_);
}

void InputIntegral::onAttachDataWatcher(ref<engine::reflow::DataWatcher> watcher_) {
	if (getDataAttributes().queryOf<attr::InputData::value>() != nullptr) {
		watcher_.attach(this, getDataAttributes().get<attr::InputData::value>());
	}
	Input::onAttachDataWatcher(watcher_);
}
void InputIntegral::onDetachDataWatcher(ref<engine::reflow::DataWatcher> watcher_) {
	if (getDataAttributes().queryOf<attr::InputData::value>() != nullptr) {
		watcher_.detach(getDataAttributes().get<attr::InputData::value>());
	}
	Input::onDetachDataWatcher(watcher_);
}

/**/

SharedPtr<InputIntegral> editor::ui::makeInputIntegral() {

	const auto content = make_sptr<uikit::Stack>();
	content->getLayoutAttributes().update<engine::reflow::attr::BoxLayout::placement>(ReflowPlacement::eMiddleLeft);
	content->addChild(make_sptr<uikit::Paint>(ReflowClassList { "[Input] > background"sv }, nullptr));

	auto text = make_sptr<uikit::Text>();
	content->addChild(clone(text));

	auto result = make_sptr<InputIntegral>(clone(content), hg::move(text));
	content->setParent(result);

	return result;
}
