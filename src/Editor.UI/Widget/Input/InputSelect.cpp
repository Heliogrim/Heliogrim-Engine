#include "InputSelect.hpp"

#include "EditorUiEventNames.hpp"
#include "Builder/MenuBuilder.hpp"
#include "Editor.UI.Main/EditorUI.hpp"
#include "Engine.Common/Discard.hpp"
#include "Engine.Reflow/DataWatcher.hpp"
#include "Engine.Reflow.Uikit/Atom/Paint.hpp"
#include "Engine.Reflow/Window/Window.hpp"

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
	_runtimeMenuHost() {}

InputSelect::~InputSelect() = default;

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

	assertd(_runtimeMenuHost == nullptr);

	const auto window = std::static_pointer_cast<Window>(root());
	assertrt(window != nullptr);

	/**/

	auto menuBuilder = MenuBuilder {};
	const auto builderEvent = SelectMenuEvent { menuBuilder };

	const auto result = invokeOnSelectMenu(builderEvent);
	::hg::assertrt(result == EventResponse::eHandled || result == EventResponse::eConsumed);

	auto menu = menuBuilder.construct();
	// TODO: Support dynamic re-positioning based on the popover content sizing
	const auto screenMenuOffset = getLayoutState().layoutOffset + math::fvec2 {
		std::max(-getLayoutState().layoutOffset.x, -300.F), getLayoutState().layoutSize.y
	};
	_runtimeMenuHost = make_sptr<Host>(menu, screenMenuOffset, math::fvec2 { std::numeric_limits<f32>::infinity() });

	discard = window->requestLayer(_runtimeMenuHost);
	menu->setParent(_runtimeMenuHost);
	menu->markAsPending(true, true);
}

void InputSelect::closeMenu() {
	const auto window = std::static_pointer_cast<Window>(root());
	assertrt(window != nullptr);

	auto host = std::exchange(_runtimeMenuHost, nullptr);
	window->dropLayer(host.get());
}

Widget::listen_handle_type InputSelect::onInputChange(listen_fn_type<InputChangeEvent> listenFn_) {
	return _emitter.on(EditorUiEventNames::InputChangeEvent, hg::move(listenFn_));
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

EventResponse InputSelect::invokeOnMouseButtonDown(ref<const MouseEvent> event_) {

	if (event_._button != 1u) {
		return Input::invokeOnMouseButtonDown(event_);
	}

	if (_runtimeMenuHost != nullptr) {
		closeMenu();
	} else {
		openMenu();
	}

	discard = Input::invokeOnMouseButtonDown(event_);
	return EventResponse::eConsumed;
}
EventResponse InputSelect::invokeOnFocus(cref<FocusEvent> event_) {
	// TODO: Check whether we still need this | Could replace mount button up
	return Input::invokeOnFocus(event_);
}

EventResponse InputSelect::invokeOnBlur(cref<FocusEvent> event_) {

	if (_runtimeMenuHost == nullptr) {
		return Input::invokeOnBlur(event_);
	}

	/**/

	closeMenu();

	discard = Input::invokeOnBlur(event_);
	return EventResponse::eHandled;
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
