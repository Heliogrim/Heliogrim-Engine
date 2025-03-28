#pragma once

#include <Engine.ACS/Traits.hpp>
#include <Engine.Common/Wrapper.hpp>

namespace hg {
	/**
	 * Forward Declaration
	 */
	class Actor;

	struct CachedActorPointer {
		/**
		 *
		 */
		ActorGuid guid = invalid_actor_guid;

		/**
		 *
		 */
		ptr<Actor> cached = nullptr;

	public:
		[[nodiscard]] constexpr ref<Actor> operator*() const noexcept {
			return *cached;
		}

		[[nodiscard]] constexpr ptr<Actor> operator->() const noexcept {
			return cached;
		}
	};
}
