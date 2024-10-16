#pragma once

#include "TypeTrait.hpp"
#include "With.hpp"

namespace hg::meta {
	template <typename Type_>
	struct WithType : public With<Type_, TypeTrait> {
		using this_type = WithType<Type_>;

		constexpr WithType() noexcept = default;

		constexpr explicit WithType(auto&&... args_) noexcept
			requires std::is_constructible_v<Type_, decltype(args_)...> :
			With<Type_, TypeTrait>(hg::forward<decltype(args_)>(args_)...) {}

		template <typename Ux_>
		constexpr explicit WithType(WithType<Ux_>&& other_) noexcept :
			With<Type_, TypeTrait>(::hg::move(other_)) {}

		constexpr WithType(this_type&&) noexcept = default;

		constexpr WithType(const this_type&) noexcept = default;

		constexpr ~WithType() noexcept = default;

	public:
		constexpr ref<this_type> operator=(this_type&&) noexcept = default;

		constexpr ref<this_type> operator=(const this_type&) noexcept = default;

	public:
		[[nodiscard]] constexpr static decltype(auto) from(auto&& val_) noexcept {
			return WithType<Type_> { WithType<std::decay_t<decltype(val_)>> { std::forward<decltype(val_)>(val_) } };
		}
	};

	WithType(auto&& arg_) -> WithType<std::remove_cvref_t<decltype(arg_)>>;
}
