#pragma once

#include <Engine.Common/Memory/SharedPointer.hpp>
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
		constexpr static event_type_id typeId { "UniverseAddedEvent"_typeId };

	public:
		explicit UniverseAddedEvent(mref<SharedPtr<::hg::engine::core::Universe>> universe_);

		UniverseAddedEvent(cref<this_type> other_) noexcept = default;

		UniverseAddedEvent(mref<this_type> other_) noexcept = default;

		~UniverseAddedEvent() override = default;

	private:
		SharedPtr<::hg::engine::core::Universe> _universe;

	public:
		[[nodiscard]] SharedPtr<hg::engine::core::Universe> getUniverse() const noexcept;
	};
}
