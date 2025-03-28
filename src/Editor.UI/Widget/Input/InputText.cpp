#include "InputText.hpp"

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

InputText::InputText(mref<SharedPtr<uikit::Stack>> content_, mref<SharedPtr<uikit::Text>> text_) :
	Input(hg::move(content_), ReflowClassList {}, nullptr),
	Atom(
		{
			InputTextDataAttributes {
				math::vec2_t<String::size_type> {
					String::size_type { 0 }, std::numeric_limits<String::size_type>::max()
				},
				String {}
			}
		},
		{},
		{}
	),
	_text(
		hg::move(text_)
	),
	_stringified() {}

InputText::~InputText() = default;

string InputText::getTag() const noexcept {
	return std::format(R"(InputText <{:#x}>)", reinterpret_cast<u64>(this));
}

bool InputText::shouldTick() const noexcept {
	return not state().isFocus() && isPristine() && state().isLayoutOrRenderPending();
}

void InputText::tick() {

	if (isTouched() && isPristine()) {
		markAsUntouched();
	}

	const auto& dataValue = getDataAttributes().valueOf<attr::InputData::value>();
	_text->setText(dataValue);
}

SharedPtr<uikit::Stack> InputText::getContent() const noexcept {
	return std::static_pointer_cast<uikit::Stack>(_children.getChild());
}

InputText::input_type InputText::value() const noexcept {
	return getDataAttributes().valueOf<attr::InputData::value>();
}

void InputText::setValue(StringView next_) {
	if (getDataAttributes().update<attr::InputData::value>(String { next_ })) {
		markAsPending(true, true);
	}
}

void InputText::updateValueAndValidity(const bool propagate_, const bool emit_) {

	auto next = StringView { _stringified };
	const auto& limits = getDataAttributes().valueOf<attr::InputData::limits>();
	if (next.size() < limits.min || limits.max < next.size()) {
		_status = InputControlStatus::eInvalid;
		return;
	}

	/**/

	_status = InputControlStatus::eValid;
	setValue(next);
	::hg::discard = _emitter.emit(EditorUiEventNames::InputChangeEvent, InputChangeEvent { shared_from_this() });

	markAsPristine();
}

Widget::listen_handle_type InputText::onInputChange(listen_fn_type<InputChangeEvent> listenFn_) {
	return _emitter.on(EditorUiEventNames::InputChangeEvent, hg::move(listenFn_));
}

EventResponse InputText::invokeOnKeyDown(ref<const KeyboardEvent> event_) {

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

	if (event_._key == /* Return */'\r' || event_._key == /* Escape */'\x1B') {

		const auto window { root() };
		const FocusEvent focusEvent { window };
		WindowManager::get()->dispatch<FocusEvent>(std::static_pointer_cast<Window, Widget>(window), focusEvent);

		updateValueAndValidity(true, true);
		return EventResponse::eConsumed;
	}

	if (event_._key == /* Backspace */'\b') {

		if (_stringified.empty()) {
			return EventResponse::eConsumed;
		}

		_stringified = _stringified.substr(0, _stringified.size() - 1);
	} else if (event_._key < 0x20 || event_._key > 0x7E) {
		return EventResponse::eConsumed;

	} else {

		if (_stringified.size() == limits.max) {
			return EventResponse::eConsumed;
		}

		if (event_._modifier & 0x40 || event_._modifier & 0x80 /* Ctrl + <> */) {
			// TODO:

		} else if (event_._modifier & 0x1 || event_._modifier & 0x2 /* Shift L/R */) {

			char code { event_._key };
			if (code >= 0x61 && code < 0x7A) {
				code -= 32;
			}

			_stringified.append(&code, 1);

		} else {
			_stringified.append(&event_._key, 1);
		}
	}

	_text->setText(_stringified);
	markAsDirty();
	return EventResponse::eConsumed;
}

EventResponse InputText::invokeOnBlur(cref<FocusEvent> event_) {
	updateValueAndValidity(true, true);
	return Input::invokeOnBlur(event_);
}

void InputText::onAttachDataWatcher(ref<engine::reflow::DataWatcher> watcher_) {
	if (getDataAttributes().queryOf<attr::InputData::value>() != nullptr) {
		watcher_.attach(this, getDataAttributes().get<attr::InputData::value>());
	}
	Input::onAttachDataWatcher(watcher_);
}
void InputText::onDetachDataWatcher(ref<engine::reflow::DataWatcher> watcher_) {
	if (getDataAttributes().queryOf<attr::InputData::value>() != nullptr) {
		watcher_.detach(getDataAttributes().get<attr::InputData::value>());
	}
	Input::onDetachDataWatcher(watcher_);
}

/**/

SharedPtr<InputText> editor::ui::makeInputText() {

	const auto content = make_sptr<uikit::Stack>();
	content->getLayoutAttributes().update<engine::reflow::attr::BoxLayout::placement>(ReflowPlacement::eMiddleLeft);
	content->addChild(make_sptr<uikit::Paint>(ReflowClassList { "[Input] > background"sv }, nullptr));

	auto text = make_sptr<uikit::Text>();
	content->addChild(clone(text));

	auto result = make_sptr<InputText>(clone(content), hg::move(text));
	content->setParent(result);

	return result;
}
