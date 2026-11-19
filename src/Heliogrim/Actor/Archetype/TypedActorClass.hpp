#pragma once

#include <Engine.Common/__macro.hpp>

#include "ActorClass.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

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
