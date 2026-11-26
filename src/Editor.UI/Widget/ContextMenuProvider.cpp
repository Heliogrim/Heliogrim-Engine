#include "ContextMenuProvider.hpp"

#include "../EditorUiEventNames.hpp"

import Editor.UI.Main;
import Editor.UI.Service;
import Engine.Common;
import Engine.Core;
import Engine.Reflect;
import Engine.Reflow;

using namespace hg::editor::ui;
using namespace hg::engine::reflow;
using namespace hg;

ContextMenuProvider::ContextMenuProvider() :
	CompoundWidget(ReflowClassList {}, nullptr),
	_menu(),
	_scope { .provider = this } {}

ContextMenuProvider::ContextMenuProvider(mref<SharedPtr<Widget>> content_) :
	CompoundWidget(::hg::move(content_), ReflowClassList {}, nullptr),
	_menu(),
	_scope { .provider = this } {}

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

ptr<void> ContextMenuProvider::providedContext(const type_id contextType_) noexcept {
	if (contextType_ == refl::ctid<ContextMenuScope>()) {
		return std::addressof(_scope);
	}
	return CompoundWidget::providedContext(contextType_);
}

EventReply ContextMenuProvider::invokeOnMouseButtonUp(ref<const MouseEvent> event_) {

	if (event_._button != 3u) {
		return CompoundWidget::invokeOnMouseButtonUp(event_);
	}

	::hg::assertrt(not _menu.has_value());

	const auto subModule = engine::Engine::getEngine()->getModules().getSubModule(EditorUIDepKey);
	::hg::assertd(subModule != nullptr);

	const auto window = std::static_pointer_cast<Window>(root());
	::hg::assertd(window != nullptr);

	const auto& service = *static_cast<const ptr<const EditorUI>>(subModule.get())->getEditorServices().contextService;

	_menu = WindowManager::get()->openPopup(
		window,
		service.buildContextMenu(*window, *this),
		PopupAnchor { .offset = event_._pointer, .size = math::vec2 {}, .side = PopupSide::eBelow, .align = PopupAlign::eStart },
		{},
		None,
		[this, self = weak_from_this()]() {
			if (not self.expired()) {
				_menu = None;
			}
		}
	);

	// Note: Invoke the basis to still call the event handlers
	::hg::discard = CompoundWidget::invokeOnMouseButtonUp(event_);
	return EventResponse::eConsumed;
}
