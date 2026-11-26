#include "ContextService.hpp"

import Editor.UI;
import Engine.Reflow;
import std;

using namespace hg::editor::ui::service;
using namespace hg::editor::ui;
using namespace hg;

ContextService::ContextService() = default;

ContextService::~ContextService() = default;

SharedPtr<Menu> ContextService::buildContextMenu(
	[[maybe_unused]] ref<const engine::reflow::Window> window_,
	ref<engine::reflow::Widget> target_
) const {

	const auto scopeAbove = [](ref<engine::reflow::Widget> widget_) -> ptr<ContextMenuProvider> {
		const auto scope = widget_.context<ContextMenuScope>();
		return scope.has_value() ? scope->provider.get() : nullptr;
	};

	auto builder = MenuBuilder {};
	auto event = engine::reflow::ContextMenuEvent { builder };

	auto* provider = dynamic_cast<ptr<ContextMenuProvider>>(std::addressof(target_));
	for (provider = provider != nullptr ? provider : scopeAbove(target_); provider != nullptr; provider = scopeAbove(*provider)) {
		if (provider->invokeOnContextMenu(event) == engine::reflow::EventResponse::eConsumed) {
			break;
		}
	}

	return builder.construct();
}
