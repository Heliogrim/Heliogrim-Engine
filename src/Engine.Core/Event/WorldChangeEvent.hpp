#pragma once

#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.Event/Event.hpp>

namespace hg::engine::core {
	class Session;
	class World;
}

namespace hg::engine::core {
	class WorldChangeEvent :
		public StatelessEvent {
	public:
		using this_type = WorldChangeEvent;

	public:
		inline static constexpr event_type_id typeId { "WorldChangeEvent"_typeId };

	public:
		WorldChangeEvent(
			nmpt<Session> session_,
			cref<sptr<::hg::engine::core::World>> prevWorld_,
			cref<sptr<::hg::engine::core::World>> nextWorld_
		);

		WorldChangeEvent(cref<this_type> other_) noexcept = default;

		WorldChangeEvent(mref<this_type> other_) noexcept = default;

		~WorldChangeEvent() override = default;

	private:
		nmpt<Session> _session;
		sptr<World> _prevWorld;
		sptr<World> _nextWorld;

	public:
		[[nodiscard]] nmpt<Session> getSession() const noexcept;

		[[nodiscard]] sptr<World> getPrevWorld() const noexcept;

		[[nodiscard]] sptr<World> getNextWorld() const noexcept;
	};
}
