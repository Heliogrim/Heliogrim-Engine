#pragma once

#include <Engine.Common/Forward.hpp>

#include "ActorClass.hpp"

namespace hg {
	/**
	 * The `ActorClass` of archetype `Type_`; the static type proves which archetype it describes.
	 */
	template <typename Type_>
	class TypedActorClass final :
		public ActorClass {
	public:
		friend class ::hg::ActorClass;

	private:
		template <typename... Args_>
		constexpr explicit TypedActorClass(Args_&&... args_) noexcept :
			ActorClass(::hg::forward<Args_>(args_)...) {}
	};
}
