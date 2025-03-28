#include "ContextService.hpp"

#include <ranges>
#include <Editor.UI/Builder/MenuBuilder.hpp>
#include <Editor.UI/Event/ContextMenuEvent.hpp>
#include <Editor.UI/Widget/ContextMenuProvider.hpp>
#include <Editor.UI/Widget/Menu.hpp>
#include <Engine.Reflow/Widget/Widget.hpp>
#include <Engine.Reflow/Window/Window.hpp>

using namespace hg::editor::ui::service;
using namespace hg::editor::ui;
using namespace hg;

ContextService::ContextService() = default;

ContextService::~ContextService() = default;

void ContextService::push(mref<SharedPtr<ContextMenuProvider>> contextMenuProvider_) {
	_pushed.emplace_back(::hg::move(contextMenuProvider_));
}

void ContextService::pop(mref<SharedPtr<ContextMenuProvider>> contextMenuProvider_) {
	_pushed.erase(std::ranges::find(_pushed, ::hg::move(contextMenuProvider_)));
}

SharedPtr<Menu> ContextService::buildContextMenu(ref<const engine::reflow::Window> window_) const {

	auto active = Vector<nmpt<const engine::reflow::Widget>> {};
	const auto& focus = window_.getFocusPath();
	active.reserve(focus.size());

	for (const auto& focused : focus) {
		const auto widget = focused.lock();
		if (focused.expired() || widget == nullptr || widget == engine::reflow::NullWidget::instance()) {
			continue;
		}

		active.emplace_back(widget.get());
	}

	/**/

	auto builder = MenuBuilder {};
	auto event = engine::reflow::ContextMenuEvent { builder };

	for (const auto& pushed : _pushed) {
		const auto pt = nmpt<const ContextMenuProvider> { pushed.get() };
		if (not std::ranges::any_of(active, [&](const auto& active_) { return active_ == pt; })) {
			continue;
		}

		auto result = pt->invokeOnContextMenu(event);
		if (result == engine::reflow::EventResponse::eConsumed) {
			break;
		}
	}

	return builder.construct();
}
