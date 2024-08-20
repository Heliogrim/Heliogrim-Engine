#pragma once

#include <type_traits>
#include <utility>

namespace hg {
	/* Note: This is a straight copy from the msvc xmemory header... */

	struct zero_then_variadic_args_t {
		constexpr explicit zero_then_variadic_args_t() = default;
	};

	struct one_then_variadic_args_t {
		constexpr explicit one_then_variadic_args_t() = default;
	};

	template <
		class FirstType_,
		class SecondType_,
		bool = std::is_empty_v<FirstType_> && not std::is_final_v<FirstType_>
	>
	class CompressedPair final : private FirstType_ {
	public:
		SecondType_ second;

		template <class... SecondArgs_>
		constexpr explicit CompressedPair(
			zero_then_variadic_args_t,
			SecondArgs_&&... secondArgs_
		) noexcept(
			std::conjunction_v<
				std::is_nothrow_default_constructible<FirstType_>,
				std::is_nothrow_constructible<SecondType_, SecondArgs_...>>
		) :
			FirstType_(),
			second(std::forward<SecondArgs_>(secondArgs_)...) {}

		template <class FirstFwd_, class... SecondArgs_>
		constexpr CompressedPair(
			one_then_variadic_args_t,
			FirstFwd_&& firstFwd_,
			SecondArgs_&&... secondArgs_
		) noexcept(
			std::conjunction_v<
				std::is_nothrow_constructible<FirstType_, FirstFwd_>,
				std::is_nothrow_constructible<SecondType_, SecondArgs_...>>
		) :
			FirstType_(std::forward<FirstFwd_>(firstFwd_)),
			second(std::forward<SecondArgs_>(secondArgs_)...) {}

		constexpr FirstType_& getFirst() noexcept {
			return *this;
		}

		constexpr const FirstType_& getFirst() const noexcept {
			return *this;
		}
	};

	template <class FirstType_, class SecondType_>
	class CompressedPair<FirstType_, SecondType_, false> final {
	public:
		FirstType_ first;
		SecondType_ second;

		template <class... SecondArgs_>
		constexpr explicit CompressedPair(
			zero_then_variadic_args_t,
			SecondArgs_&&... secondArgs_
		) noexcept(
			std::conjunction_v<
				std::is_nothrow_default_constructible<FirstType_>,
				std::is_nothrow_constructible<SecondType_, SecondArgs_...>>
		) :
			first(),
			second(std::forward<SecondArgs_>(secondArgs_)...) {}

		template <class FirstFwd_, class... SecondArgs_>
		constexpr CompressedPair(
			one_then_variadic_args_t,
			FirstFwd_&& firstFwd_,
			SecondArgs_&&... secondArgs_
		) noexcept(
			std::conjunction_v<
				std::is_nothrow_constructible<FirstType_, FirstFwd_>,
				std::is_nothrow_constructible<SecondType_, SecondArgs_...>>
		) :
			first(std::forward<FirstFwd_>(firstFwd_)),
			second(std::forward<SecondArgs_>(secondArgs_)...) {}

		constexpr FirstType_& getFirst() noexcept {
			return first;
		}

		constexpr const FirstType_& getFirst() const noexcept {
			return first;
		}
	};
}
