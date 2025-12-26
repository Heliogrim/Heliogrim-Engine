#pragma once

#include <Engine.Event/Event.hpp>
#include "../Builder/MenuBuilder.hpp"

namespace hg::engine::reflow {
	class SelectMenuEvent :
		public StatelessEvent {
	public:
		using this_type = SelectMenuEvent;

	public:
		inline static constexpr event_type_id typeId { "SelectMenuEvent"_typeId };

	public:
		explicit SelectMenuEvent(ref<editor::ui::MenuBuilder> builder_) noexcept;

		~SelectMenuEvent() noexcept override = default;

	public:
		ref<editor::ui::MenuBuilder> builder;
	};
}
