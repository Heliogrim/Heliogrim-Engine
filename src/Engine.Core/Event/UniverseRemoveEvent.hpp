#pragma once

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
		inline static constexpr event_type_id typeId { "UniverseRemoveEvent"_typeId };

	public:
		explicit UniverseRemoveEvent(cref<sptr<::hg::engine::core::Universe>> universe_);

		UniverseRemoveEvent(cref<this_type> other_) noexcept = default;

		UniverseRemoveEvent(mref<this_type> other_) noexcept = default;

		~UniverseRemoveEvent() override = default;

	private:
		sptr<::hg::engine::core::Universe> _universe;

	public:
		[[nodiscard]] sptr<hg::engine::core::Universe> getUniverse() const noexcept;
	};
}
