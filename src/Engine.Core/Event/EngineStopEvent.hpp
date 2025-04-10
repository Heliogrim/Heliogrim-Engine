#pragma once

#include <Engine.Event/Event.hpp>

namespace hg::engine::core {
	class EngineStopEvent final :
		public StatelessEvent {
	public:
		using this_type = EngineStopEvent;

	public:
		inline static constexpr event_type_id typeId { "EngineStopEvent"_typeId };

	public:
		constexpr EngineStopEvent() noexcept = default;

		constexpr ~EngineStopEvent() noexcept override = default;
	};
}
