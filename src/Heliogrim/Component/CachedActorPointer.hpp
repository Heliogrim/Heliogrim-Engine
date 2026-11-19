#pragma once

#include <Engine.ACS/Traits.hpp>
#include <Engine.Common/__macro.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

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
