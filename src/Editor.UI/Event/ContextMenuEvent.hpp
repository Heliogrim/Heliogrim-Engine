#pragma once

#include "../Builder/MenuBuilder.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Event;
#endif

namespace hg::engine::reflow {
	class ContextMenuEvent :
		public StatelessEvent {
	public:
		using this_type = ContextMenuEvent;

	public:
		inline static constexpr event_type_id typeId { "ContextMenuEvent"_typeId };

	public:
		explicit ContextMenuEvent(ref<editor::ui::MenuBuilder> builder_) noexcept;

		~ContextMenuEvent() noexcept override = default;

	public:
		ref<editor::ui::MenuBuilder> builder;
	};
}
