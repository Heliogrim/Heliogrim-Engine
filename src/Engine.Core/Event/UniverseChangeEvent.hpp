#pragma once

#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.Event/Event.hpp>

namespace hg::engine::core {
	class Session;
	class Universe;
}

namespace hg::engine::core {
	class UniverseChangeEvent :
		public StatelessEvent {
	public:
		using this_type = UniverseChangeEvent;

	public:
		inline static constexpr event_type_id typeId { "UniverseChangeEvent"_typeId };

	public:
		UniverseChangeEvent(
			nmpt<Session> session_,
			cref<sptr<::hg::engine::core::Universe>> prevUniverse_,
			cref<sptr<::hg::engine::core::Universe>> nextUniverse_
		);

		UniverseChangeEvent(cref<this_type> other_) noexcept = default;

		UniverseChangeEvent(mref<this_type> other_) noexcept = default;

		~UniverseChangeEvent() override = default;

	private:
		nmpt<Session> _session;
		sptr<Universe> _prevUniverse;
		sptr<Universe> _nextUniverse;

	public:
		[[nodiscard]] nmpt<Session> getSession() const noexcept;

		[[nodiscard]] sptr<Universe> getPrevUniverse() const noexcept;

		[[nodiscard]] sptr<Universe> getNextUniverse() const noexcept;
	};
}
