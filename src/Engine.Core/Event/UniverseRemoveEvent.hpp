#pragma once

#include <Engine.Common/Memory/SharedPointer.hpp>
#include <Engine.Event/Event.hpp>

namespace hg::engine::core {
	class Universe;
}

namespace hg::engine::core {
	class UniverseRemoveEvent :
		public StatelessEvent {
	public:
		using this_type = UniverseRemoveEvent;

	public:
		constexpr static event_type_id typeId { "UniverseRemoveEvent"_typeId };

	public:
		explicit UniverseRemoveEvent(mref<SharedPtr<::hg::engine::core::Universe>> universe_);

		UniverseRemoveEvent(cref<this_type> other_) noexcept = default;

		UniverseRemoveEvent(mref<this_type> other_) noexcept = default;

		~UniverseRemoveEvent() override = default;

	private:
		SharedPtr<::hg::engine::core::Universe> _universe;

	public:
		[[nodiscard]] SharedPtr<hg::engine::core::Universe> getUniverse() const noexcept;
	};
}
