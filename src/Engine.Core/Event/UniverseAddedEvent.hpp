#pragma once

#include <Engine.Event/Event.hpp>

namespace hg::engine::core {
	class Universe;
}

namespace hg::engine::core {
	class UniverseAddedEvent :
		public StatelessEvent {
	public:
		using this_type = UniverseAddedEvent;

	public:
		inline static constexpr event_type_id typeId { "UniverseAddedEvent"_typeId };

	public:
		explicit UniverseAddedEvent(cref<sptr<::hg::engine::core::Universe>> universe_);

		UniverseAddedEvent(cref<this_type> other_) noexcept = default;

		UniverseAddedEvent(mref<this_type> other_) noexcept = default;

		~UniverseAddedEvent() override = default;

	private:
		sptr<::hg::engine::core::Universe> _universe;

	public:
		[[nodiscard]] sptr<hg::engine::core::Universe> getUniverse() const noexcept;
	};
}
