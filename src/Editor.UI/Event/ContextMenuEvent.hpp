#pragma once

#include <Engine.Event/Event.hpp>
#include "../Builder/MenuBuilder.hpp"

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
