#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Event;
#endif

namespace hg::engine::core {
	class EnginePreInitEvent final :
		public StatelessEvent {
	public:
		using this_type = EnginePreInitEvent;

	public:
		inline static constexpr event_type_id typeId { "EnginePreInitEvent"_typeId };

	public:
		constexpr EnginePreInitEvent() noexcept = default;

		constexpr ~EnginePreInitEvent() noexcept override = default;
	};
}
