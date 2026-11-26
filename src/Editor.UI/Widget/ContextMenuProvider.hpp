#pragma once

#include "../Event/ContextMenuEvent.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Reflow;
#endif

namespace hg::editor::ui {
	class ContextMenuProvider;

	/**
	 * Nearest context-menu provider above a widget; right click collects these from the clicked widget upward.
	 */
	struct ContextMenuScope {
		constexpr static bool isStatic = true;
		nmpt<ContextMenuProvider> provider;
	};

	class ContextMenuProvider :
		public engine::reflow::CompoundWidget {
	public:
		using this_type = ContextMenuProvider;

		using ContextMenuEvent = ::hg::engine::reflow::ContextMenuEvent;

	public:
		ContextMenuProvider();

		explicit ContextMenuProvider(mref<SharedPtr<Widget>> content_);

		~ContextMenuProvider() override;

	public:
		[[nodiscard]] string getTag() const noexcept override;

	private:
		Opt<engine::reflow::PopupHandle> _menu;
		ContextMenuScope _scope;

	public:
		void setContent(mref<SharedPtr<Widget>> content_);

		engine::reflow::EventResponse invokeOnContextMenu(ref<const ContextMenuEvent> event_) const;

		listen_handle_type onContextMenu(listen_fn_type<ContextMenuEvent> listenFn_);

		bool dropOnContextMenu(listen_handle_type handle_);

		[[nodiscard]] ptr<void> providedContext(const type_id contextType_) noexcept override;

	public:
		engine::reflow::EventReply invokeOnMouseButtonUp(ref<const engine::reflow::MouseEvent> event_) override;
	};
}
