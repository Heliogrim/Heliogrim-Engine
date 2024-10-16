#pragma once

#include <concepts>
#include <type_traits>

#include <Engine.Common/Move.hpp>

namespace hg::meta {
	template <typename Trait_>
	concept IsWithTrait = requires(const Trait_& obj_) {
		{ obj_.valid() } -> std::same_as<bool>;
	};

	template <typename Type_, template <typename> typename... Traits_>
		requires std::conjunction_v<std::bool_constant<IsWithTrait<Traits_<Type_>>>...>
	struct With : Traits_<Type_>... {
		using this_type = With<Type_, Traits_...>;

		Type_ value;

		constexpr With() noexcept = default;

		constexpr explicit With(auto&&... args_) noexcept
			requires std::is_constructible_v<Type_, decltype(args_)...> :
			Traits_<Type_> {}...,
			value(std::forward<decltype(args_)>(args_)...) {}

		template <typename Ux_>
		constexpr explicit With(With<Ux_, Traits_...>&& other_) noexcept :
			Traits_<Type_> { static_cast<Traits_<Ux_>&&>(other_) }...,
			value(::hg::move(other_.value)) {}

		constexpr With(this_type&&) noexcept = default;

		constexpr With(const this_type&) noexcept = default;

		constexpr ~With() noexcept = default;

	public:
		ref<this_type> operator=(this_type&&) noexcept = default;

		ref<this_type> operator=(const this_type&) noexcept = default;

	public:
		[[nodiscard]] constexpr bool valid() const noexcept {
			auto val = true;
			((val = val && static_cast<const Traits_<Type_>&>(*this).valid()), ...);
			return val;
		}
	};
}
