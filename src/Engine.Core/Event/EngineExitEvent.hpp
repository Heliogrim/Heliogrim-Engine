#pragma once

#include <Engine.Event/Event.hpp>

namespace hg::engine::core {
	/**
	 * Warning: We might not be able to handle or dispatch this event due to the main engine lifecycle
	 *  When shutdown is complete, scheduler is not guaranteed to dispatch any more jobs, beside the destruction of dependency chains.
	 *  Furthermore the lifecycle of the emitter and all event listeners might be tight to the engine object lifecycle,
	 *  which might result in a use-after-free scenario.
	 */
	class EngineExitEvent final :
		public StatelessEvent {
	public:
		using this_type = EngineExitEvent;

	public:
		inline static constexpr event_type_id typeId { "EngineExitEvent"_typeId };

	public:
		constexpr EngineExitEvent() noexcept = default;

		constexpr ~EngineExitEvent() noexcept override = default;
	};
}
