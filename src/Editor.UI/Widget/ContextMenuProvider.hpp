#pragma once

#include <Engine.Reflow/Widget/CompoundWidget.hpp>

#include "../Event/ContextMenuEvent.hpp"

namespace hg::engine::reflow {
	class Host;
}

namespace hg::editor::ui {
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
		SharedPtr<engine::reflow::Host> _runtimeMenuHost;

	public:
		void setContent(mref<SharedPtr<Widget>> content_);

		engine::reflow::EventResponse invokeOnContextMenu(ref<const ContextMenuEvent> event_) const;

		listen_handle_type onContextMenu(listen_fn_type<ContextMenuEvent> listenFn_);

		bool dropOnContextMenu(listen_handle_type handle_);

	public:
		engine::reflow::EventResponse invokeOnFocus(cref<engine::reflow::FocusEvent> event_) override;

		engine::reflow::EventResponse invokeOnBlur(cref<engine::reflow::FocusEvent> event_) override;

		engine::reflow::EventResponse invokeOnMouseButtonUp(ref<const engine::reflow::MouseEvent> event_) override;
	};
}
