#pragma once

#include <Engine.Event/Event.hpp>

namespace hg::engine::core {
	class EnginePostInitEvent final :
		public StatelessEvent {
	public:
		using this_type = EnginePostInitEvent;

	public:
		inline static constexpr event_type_id typeId { "EnginePostInitEvent"_typeId };

	public:
		constexpr EnginePostInitEvent() noexcept = default;

		constexpr ~EnginePostInitEvent() noexcept override = default;
	};
}
