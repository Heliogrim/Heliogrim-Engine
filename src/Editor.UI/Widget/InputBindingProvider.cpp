#include "InputBindingProvider.hpp"

#include <format>
#include <Editor.Action/ActionManager.hpp>
#include <Editor.UI.Main/EditorUI.hpp>
#include <Editor.UI.Main/Module/EditorUI.hpp>
#include <Engine.Core/Module/Modules.hpp>
#include <Engine.Logging/Logger.hpp>
#include <Engine.Reflow/Window/Window.hpp>
#include <Engine.Reflow/Window/WindowManager.hpp>
#include <SDL3/SDL_keycode.h>

using namespace hg::editor::ui;
using namespace hg::engine::reflow;
using namespace hg;

/**/

static Opt<ref<const service::InputBindingList>> resolveInputBindings(ref<const String> key_);

/**/

InputBindingProvider::InputBindingProvider() :
	CompoundWidget(ReflowClassList {}, nullptr),
	_input_binding_key(service::general_input_binding_list),
	_input_bindings(resolveInputBindings(_input_binding_key)) {}

InputBindingProvider::InputBindingProvider(mref<SharedPtr<Widget>> content_) :
	CompoundWidget(::hg::move(content_), ReflowClassList {}, nullptr) {}

InputBindingProvider::~InputBindingProvider() = default;

string InputBindingProvider::getTag() const noexcept {
	return std::format(R"(InputBindingProvider <{:#x}>)", reinterpret_cast<u64>(this));
}

void InputBindingProvider::setContent(mref<SharedPtr<Widget>> content_) {
	content_->setParent(shared_from_this());
	auto prev = _children.setChild(::hg::move(content_));
	prev->setParent(nullptr);
	markAsPending(true, true);
}

engine::reflow::EventResponse InputBindingProvider::invokeOnBlur(cref<engine::reflow::FocusEvent> event_) {
	_state.unset(WidgetStateFlagBits::eFocus);
	// Note: markAsPending(false, false);
	return CompoundWidget::invokeOnBlur(event_);
}
engine::reflow::EventResponse InputBindingProvider::invokeOnFocus(cref<engine::reflow::FocusEvent> event_) {
	_state.set(WidgetStateFlagBits::eFocus);
	// Note: markAsPending(false, false);
	return CompoundWidget::invokeOnFocus(event_);
}

engine::reflow::EventResponse InputBindingProvider::invokeOnKeyDown(ref<const engine::reflow::KeyboardEvent> event_) {

	if (_input_bindings == None) {
		return CompoundWidget::invokeOnKeyDown(event_);
	}

	auto currentKeySet = InlineAutoArray<u32, 4> {};
	if (event_._modifier & SDL_KMOD_CTRL) {
		currentKeySet.emplace_back(SDL_KMOD_CTRL);
	}
	if (event_._modifier & SDL_KMOD_SHIFT) {
		currentKeySet.emplace_back(SDL_KMOD_SHIFT);
	}
	currentKeySet.emplace_back(event_._key);

	auto maybeAction = _input_bindings->invoke(currentKeySet);
	if (maybeAction == None) {
		return CompoundWidget::invokeOnKeyDown(event_);
	}

	const auto module = engine::Engine::getEngine()->getModules().getSubModule(editor::ActionDepKey);
	if (module == nullptr) {
		IM_DEBUG_LOG("Captured user input at input binding provider without action handler module present.");
		return EventResponse::eConsumed;
	}

	static_cast<ptr<ActionManager>>(module.get())->apply(::hg::move(maybeAction).value());
	return EventResponse::eConsumed;
}

/**/

Opt<ref<const service::InputBindingList>> resolveInputBindings(ref<const String> key_) {
	const auto subModule = engine::Engine::getEngine()->getModules().getSubModule(editor::EditorUIDepKey);
	return subModule != nullptr ?
		static_cast<const ptr<const editor::EditorUI>>(subModule.get())->getEditorServices().inputService->getInputBindings(key_) :
		None;
}
