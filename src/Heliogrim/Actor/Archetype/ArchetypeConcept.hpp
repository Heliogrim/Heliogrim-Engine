#pragma once

#include <concepts>

#include "TypedActorClass.hpp"

namespace hg {
	/**
	 * A class deriving with plain `InheritMeta` inherits its parent's `actorClass()` and fails the typed return.
	 */
	template <typename Type_>
	concept IsArchetype = requires {
		{ Type_::actorClass() } -> std::same_as<const TypedActorClass<Type_>&>;
		typename Type_::layout;
	};

	/**
	 * Whether `Type_` can tick, own or inherited (spec 6.3/6.4: a base's plain body serves derived archetypes).
	 * Decided from the type, never by comparing member pointers (spec 6.5).
	 */
	template <typename Type_>
	inline constexpr bool has_tick_v = requires(Type_& actor_) { actor_.tick(); };
}
