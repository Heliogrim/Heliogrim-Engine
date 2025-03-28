#include "ContextMenuProvider.hpp"

#include <format>
#include <Editor.UI.Main/EditorUI.hpp>
#include <Editor.UI.Main/Module/EditorUI.hpp>
#include <Engine.Common/Discard.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Core/Module/Modules.hpp>
#include <Engine.Reflow/Window/Window.hpp>
#include <Engine.Reflow/Window/WindowManager.hpp>

#include "../EditorUiEventNames.hpp"

using namespace hg::editor::ui;
using namespace hg::engine::reflow;
using namespace hg;

ContextMenuProvider::ContextMenuProvider() :
	CompoundWidget(ReflowClassList {}, nullptr),
	_runtimeMenuHost() {}

ContextMenuProvider::ContextMenuProvider(mref<SharedPtr<Widget>> content_) :
	CompoundWidget(::hg::move(content_), ReflowClassList {}, nullptr),
	_runtimeMenuHost() {}

ContextMenuProvider::~ContextMenuProvider() = default;

string ContextMenuProvider::getTag() const noexcept {
	return std::format(R"(ContextMenuProvider <{:#x}>)", reinterpret_cast<u64>(this));
}

void ContextMenuProvider::setContent(mref<SharedPtr<Widget>> content_) {
	content_->setParent(shared_from_this());
	auto prev = _children.setChild(::hg::move(content_));
	prev->setParent(nullptr);
	markAsPending(true, true);
}

EventResponse ContextMenuProvider::invokeOnContextMenu(ref<const ContextMenuEvent> event_) const {
	return _emitter.emit(EditorUiEventNames::ContextMenuEvent, event_);
}

Widget::listen_handle_type ContextMenuProvider::onContextMenu(listen_fn_type<ContextMenuEvent> listenFn_) {
	return _emitter.on(EditorUiEventNames::ContextMenuEvent, ::hg::move(listenFn_));
}

bool ContextMenuProvider::dropOnContextMenu(listen_handle_type handle_) {
	return _emitter.off<ContextMenuEvent>(EditorUiEventNames::ContextMenuEvent, ::hg::move(handle_));
}

EventResponse ContextMenuProvider::invokeOnFocus(cref<FocusEvent> event_) {

	const auto subModule = engine::Engine::getEngine()->getModules().getSubModule(EditorUIDepKey);
	::hg::assertd(subModule != nullptr);
	static_cast<const ptr<const EditorUI>>(subModule.get())->getEditorServices().contextService->push(
		std::static_pointer_cast<ContextMenuProvider>(shared_from_this())
	);

	return CompoundWidget::invokeOnFocus(event_);
}
EventResponse ContextMenuProvider::invokeOnBlur(cref<FocusEvent> event_) {

	const auto subModule = engine::Engine::getEngine()->getModules().getSubModule(EditorUIDepKey);
	::hg::assertd(subModule != nullptr);
	static_cast<const ptr<const EditorUI>>(subModule.get())->getEditorServices().contextService->pop(
		std::static_pointer_cast<ContextMenuProvider>(shared_from_this())
	);

	return CompoundWidget::invokeOnBlur(event_);
}
EventResponse ContextMenuProvider::invokeOnMouseButtonUp(ref<const MouseEvent> event_) {

	if (event_._button != 3u) {
		return CompoundWidget::invokeOnMouseButtonUp(event_);
	}

	::hg::assertrt(_runtimeMenuHost == nullptr);

	const auto subModule = engine::Engine::getEngine()->getModules().getSubModule(EditorUIDepKey);
	::hg::assertd(subModule != nullptr);

	const auto window = std::static_pointer_cast<Window>(root());
	::hg::assertd(window != nullptr);

	const auto& service = *static_cast<const ptr<const EditorUI>>(subModule.get())->getEditorServices().contextService;

	// TODO: Update positioning and possible sizing constraints for host
	const auto menu = service.buildContextMenu(*window);
	_runtimeMenuHost = make_sptr<Host>(
		menu,
		event_._pointer,
		math::fvec2 { std::numeric_limits<f32>::infinity() }
	);
	[[maybe_unused]] auto layer = window->requestLayer(_runtimeMenuHost);
	menu->setParent(_runtimeMenuHost);

	/**/

	menu->markAsPending(true, true);
	auto focusEvent = FocusEvent { menu };
	WindowManager::get()->dispatch(window, focusEvent);

	/**/

	::hg::discard = _runtimeMenuHost->onBlur(
		[this, window, self = shared_from_this()]([[maybe_unused]] ref<const FocusEvent> event_) {
			auto host = std::exchange(_runtimeMenuHost, nullptr);
			window->dropLayer(host.get());
			return EventResponse::eHandled;
		}
	);

	/**/

	// Note: Invoke the basis to still call the event handlers
	::hg::discard = CompoundWidget::invokeOnMouseButtonUp(event_);
	return EventResponse::eConsumed;
}
