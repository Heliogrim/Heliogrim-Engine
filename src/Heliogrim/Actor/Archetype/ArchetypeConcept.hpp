#pragma once

#include <concepts>
#include <type_traits>

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
}

namespace hg::archetype_detail {
	template <typename>
	struct member_class;

	template <typename Member_, typename Class_>
	struct member_class<Member_ Class_::*> {
		using type = Class_;
	};

	template <typename>
	struct first_param;

	template <typename Ret_, typename First_, typename... Rest_>
	struct first_param<Ret_ (*)(First_, Rest_...)> {
		using type = First_;
	};

	template <typename Ret_, typename First_, typename... Rest_>
	struct first_param<Ret_ (*)(First_, Rest_...) noexcept> {
		using type = First_;
	};

	/**
	 * Whether `Type_` itself declares `tick`, decided from types (spec 6.5), never by comparing member pointers.
	 *
	 * - `void tick()`: the member pointer's class is the declaring class.
	 * - `void tick(this Type_&)`: the explicit object parameter names the class.
	 * - `void tick(this auto& self)` (void return): lookup on `Type_` finds a different specialization than on its archetype
	 *	base. Explicit object functions are plain functions, so this compares function pointers ([expr.eq]/3),
	 *	which is well-defined, unlike virtual member pointers ([expr.eq]/5.3).
	 */
	template <typename Type_>
	[[nodiscard]] consteval bool declares_tick() {
		if constexpr (requires { &Type_::tick; }) {
			using member = decltype(&Type_::tick);
			if constexpr (std::is_member_function_pointer_v<member>) {
				return std::is_same_v<typename member_class<member>::type, Type_>;
			} else {
				return std::is_same_v<std::remove_cvref_t<typename first_param<member>::type>, Type_>;
			}
		} else if constexpr (requires { &Type_::template tick<Type_>; }) {
			using base = typename Type_::archetype_base_type;
			if constexpr (requires { &base::template tick<Type_>; }) {
				// Typed targets: MSVC cannot resolve the template addresses as bare `!=` operands.
				using fn = void (*)(Type_&);
				constexpr fn own = &Type_::template tick<Type_>;
				constexpr fn inherited = &base::template tick<Type_>;
				return own != inherited;
			} else {
				return true;
			}
		} else {
			return false;
		}
	}
}

namespace hg {
	template <typename Type_>
	inline constexpr bool has_tick_v = archetype_detail::declares_tick<Type_>();
}
