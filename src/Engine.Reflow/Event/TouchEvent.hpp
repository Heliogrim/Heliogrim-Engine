#pragma once

#include <Engine.Event/Event.hpp>

namespace hg::engine::reflow {
	class Widget;
}

namespace hg::engine::reflow {
	class TouchEvent :
		public StatelessEvent {
	public:
		using this_type = TouchEvent;
		using reference_type = ref<this_type>;
		using const_reference_type = cref<this_type>;

	public:
		inline static constexpr event_type_id typeId { "TouchEvent"_typeId };

	public:
		constexpr explicit TouchEvent() noexcept = default;

		constexpr ~TouchEvent() noexcept override = default;

	private:
	public:
	};
}
