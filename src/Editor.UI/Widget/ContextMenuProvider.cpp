#include "ContextMenuProvider.hpp"

#include <format>
#include <Editor.UI.Main/EditorUI.hpp>
#include <Editor.UI.Main/Module/EditorUI.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Core/Module/Modules.hpp>
#include <Engine.Reflow/Window/Window.hpp>

#include "../EditorUiEventNames.hpp"

using namespace hg::editor::ui;
using namespace hg::engine::reflow;
using namespace hg;

ContextMenuProvider::ContextMenuProvider() :
	CompoundWidget(ReflowClassList {}, nullptr) {}

ContextMenuProvider::ContextMenuProvider(mref<SharedPtr<Widget>> content_) :
	CompoundWidget(::hg::move(content_), ReflowClassList {}, nullptr) {}

ContextMenuProvider::~ContextMenuProvider() = default;

string ContextMenuProvider::getTag() const noexcept {
	return std::format(R"(ContextMenuProvider <{:#x}>)", reinterpret_cast<u64>(this));
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

	if (event_._button != 2u) {
		return CompoundWidget::invokeOnMouseButtonUp(event_);
	}

	const auto subModule = engine::Engine::getEngine()->getModules().getSubModule(EditorUIDepKey);
	::hg::assertd(subModule != nullptr);

	const auto root = this->root();
	::hg::assertd(root != nullptr);

	auto& window = *static_cast<const ptr<Window>>(root.get());
	const auto& service = *static_cast<const ptr<const EditorUI>>(subModule.get())->getEditorServices().contextService;

	return CompoundWidget::invokeOnMouseButtonUp(event_);
}
