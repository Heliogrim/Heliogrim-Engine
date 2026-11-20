#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Event;
#endif

namespace hg::engine::core {
	class EngineShutdownEvent final :
		public StatelessEvent {
	public:
		using this_type = EngineShutdownEvent;

	public:
		inline static constexpr event_type_id typeId { "EngineShutdownEvent"_typeId };

	public:
		constexpr EngineShutdownEvent() noexcept = default;

		constexpr ~EngineShutdownEvent() noexcept override = default;
	};
}
