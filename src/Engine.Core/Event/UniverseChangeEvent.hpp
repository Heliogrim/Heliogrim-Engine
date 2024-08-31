#pragma once

#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.Common/Memory/SharedPointer.hpp>
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
		constexpr static event_type_id typeId { "UniverseChangeEvent"_typeId };

	public:
		UniverseChangeEvent(
			nmpt<Session> session_,
			mref<SharedPtr<::hg::engine::core::Universe>> prevUniverse_,
			mref<SharedPtr<::hg::engine::core::Universe>> nextUniverse_
		);

		UniverseChangeEvent(cref<this_type> other_) noexcept = default;

		UniverseChangeEvent(mref<this_type> other_) noexcept = default;

		~UniverseChangeEvent() override = default;

	private:
		nmpt<Session> _session;
		SharedPtr<Universe> _prevUniverse;
		SharedPtr<Universe> _nextUniverse;

	public:
		[[nodiscard]] nmpt<Session> getSession() const noexcept;

		[[nodiscard]] SharedPtr<Universe> getPrevUniverse() const noexcept;

		[[nodiscard]] SharedPtr<Universe> getNextUniverse() const noexcept;
	};
}
