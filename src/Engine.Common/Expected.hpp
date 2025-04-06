#pragma once

#include <tl/expected.hpp>

#include "Forward.hpp"

namespace hg {
	/**/
	template <typename Type_>
	struct Expected;

	template <typename Error_>
	struct Unexpected;

	/**/

	template <typename Type_, typename Error_>
	class Result :
		public tl::expected<Type_, Error_> {
	public:
		using this_type = Result<Type_, Error_>;
		using underlying_type = tl::expected<Type_, Error_>;

		using value_type = typename underlying_type::value_type;

	public:
		using underlying_type::expected;

	public:
		constexpr Result(Expected<Type_>&& expected_) noexcept :// NOLINT(*-explicit-constructor)
			underlying_type(std::move(expected_.value)) {}

		constexpr Result(Unexpected<Error_>&& unexpected_) noexcept:// NOLINT(*-explicit-constructor)
			underlying_type(::tl::unexpected { std::move(unexpected_.error) }) {}

		constexpr Result(underlying_type&& other_) noexcept :
			underlying_type(::hg::forward<decltype(other_)>(other_)) {}

	public:
		constexpr static decltype(auto) expected(auto&&... args_) noexcept
			requires std::is_constructible_v<this_type, decltype(args_)...> {
			return this_type { std::forward<decltype(args_)>(args_)... };
		}

		constexpr static decltype(auto) unexpected(auto&&... error_) noexcept
			requires std::is_constructible_v<Error_, decltype(error_)...> {
			return this_type { ::tl::make_unexpected(std::forward<decltype(error_)>(error_)...) };
		}
	};

	/**/

	template <typename Type_>
	struct Expected {
		Type_ value;

		constexpr explicit Expected(auto&&... args_) noexcept :
			value(std::forward<decltype(args_)>(args_)...) {}
	};

	template <>
	struct Expected<void> {
		constexpr explicit Expected(auto&&...) noexcept {}
	};

	template <typename Error_>
	struct Unexpected {
		Error_ error;

		constexpr explicit Unexpected(auto&&... args_) noexcept :
			error(std::forward<decltype(args_)>(args_)...) {}
	};

	/**/

	template <typename Type_>
	Expected(Type_&&) -> Expected<Type_>;

	template <typename Error_>
	Unexpected(Error_&&) -> Unexpected<std::decay_t<Error_>>;
}

/**/

template <class T, class E>
struct tl::detail::is_expected_impl<hg::Result<T, E>> : std::true_type {};
