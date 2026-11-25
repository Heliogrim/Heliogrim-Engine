#include "InputSelect.hpp"

#include "EditorUiEventNames.hpp"
#include "Builder/MenuBuilder.hpp"

import Engine.Common;
import Engine.Reflow;
import Engine.Reflow.Uikit;

using namespace hg::editor::ui;
using namespace hg::engine::reflow;
using namespace hg;

InputSelect::InputSelect(
	mref<SharedPtr<uikit::Stack>> content_,
	mref<SharedPtr<uikit::Text>> text_
) :
	Input(hg::move(content_), ReflowClassList {}, nullptr),
	Atom({}, {}, {}),
	_text(hg::move(text_)),
	_menu() {
	bindAttributeOwner(this);
}

InputSelect::~InputSelect() {
	closeMenu();
}

string InputSelect::getTag() const noexcept {
	return std::format(R"(InputSelect <{:#x}>)", reinterpret_cast<u64>(this));
}

bool InputSelect::shouldTick() const noexcept {
	return not state().isFocus() && isPristine() && state().isLayoutOrRenderPending();
}

void InputSelect::tick() {

	if (isTouched() && isPristine()) {
		markAsUntouched();
	}

	const auto& dataValue = getDataAttributes().valueOf<attr::InputData::value>();
	_text->setText(dataValue);
}

void InputSelect::openMenu() {

	assertd(not _menu.has_value());

	const auto window = std::static_pointer_cast<Window>(root());
	assertrt(window != nullptr);

	/**/

	auto menuBuilder = MenuBuilder {};
	const auto builderEvent = SelectMenuEvent { menuBuilder };

	const auto result = invokeOnSelectMenu(builderEvent);
	::hg::assertrt(result == EventResponse::eHandled || result == EventResponse::eConsumed);

	const auto& layout = getLayoutState();
	_menu = WindowManager::get()->openPopup(
		window,
		menuBuilder.construct(),
		PopupAnchor { .offset = layout.layoutOffset, .size = layout.layoutSize, .side = PopupSide::eBelow, .align = PopupAlign::eStart },
		weak_from_this(),
		None,
		[this, self = weak_from_this()]() {
			if (not self.expired()) {
				_menu = None;
			}
		}
	);
}

void InputSelect::closeMenu() {
	// The manager is gone at shutdown, its router drops the popups itself.
	if (const auto& manager = WindowManager::get(); _menu.has_value() && manager != nullptr) {
		manager->closePopup(*_menu);
	}
}

Widget::listen_handle_type InputSelect::onInputChange(listen_fn_type<InputChangeEvent> listenFn_) {
	return _emitter.on(ReflowEventNames::InputChangeEvent, hg::move(listenFn_));
}

Widget::listen_handle_type InputSelect::onSelectMenu(listen_fn_type<SelectMenuEvent> listenFn_) {
	return _emitter.on(EditorUiEventNames::SelectMenuEvent, ::hg::move(listenFn_));
}

bool InputSelect::dropOnSelectMenu(listen_handle_type handle_) {
	return _emitter.off<SelectMenuEvent>(EditorUiEventNames::SelectMenuEvent, ::hg::move(handle_));
}

EventResponse InputSelect::invokeOnSelectMenu(ref<const SelectMenuEvent> event_) const {
	return _emitter.emit(EditorUiEventNames::SelectMenuEvent, event_);
}

EventReply InputSelect::invokeOnMouseButtonDown(ref<const MouseEvent> event_) {

	if (event_._button != 1u) {
		return Input::invokeOnMouseButtonDown(event_);
	}

	if (_menu.has_value()) {
		closeMenu();
	} else {
		openMenu();
	}

	discard = Input::invokeOnMouseButtonDown(event_);
	return EventResponse::eConsumed;
}
EventReply InputSelect::invokeOnFocus(cref<FocusEvent> event_) {
	// TODO: Check whether we still need this | Could replace mount button up
	return Input::invokeOnFocus(event_);
}

void InputSelect::onAttachDataWatcher(ref<DataWatcher> watcher_) {
	if (getDataAttributes().queryOf<attr::InputData::value>() != nullptr) {
		watcher_.attach(this, getDataAttributes().get<attr::InputData::value>());
	}
	Input::onAttachDataWatcher(watcher_);
}

void InputSelect::onDetachDataWatcher(ref<DataWatcher> watcher_) {
	if (getDataAttributes().queryOf<attr::InputData::value>() != nullptr) {
		watcher_.detach(getDataAttributes().get<attr::InputData::value>());
	}
	Input::onDetachDataWatcher(watcher_);
}

void InputSelect::updateValueAndValidity(bool propagate_, bool emit_) {
	Input<std::string>::updateValueAndValidity(propagate_, emit_);
}

Input<std::string>::input_type InputSelect::value() const noexcept {
	return getDataAttributes().valueOf<attr::InputData::value>();
}

SharedPtr<InputSelect> editor::ui::makeInputSelect() {

	const auto content = make_sptr<uikit::Stack>();
	content->getLayoutAttributes().update<engine::reflow::attr::BoxLayout::placement>(ReflowPlacement::eMiddleLeft);
	content->addChild(make_sptr<uikit::Paint>(ReflowClassList { "[Input] > background"sv }, nullptr));

	auto text = make_sptr<uikit::Text>();
	content->addChild(clone(text));

	auto result = make_sptr<InputSelect>(clone(content), hg::move(text));
	content->setParent(result);

	return result;
}
