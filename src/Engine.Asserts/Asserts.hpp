#pragma once

#include <stdexcept>
#include <type_traits>
#include <Engine.Common/__macro.hpp>

#include "Breakpoint.hpp"
#include "Panic.hpp"

namespace hg {
	struct assert_compile_error final {
		const char* msg = "Assert failed at compile time.";
	};

	/**/

	START_SUPPRESS_WARNINGS

	inline void throw_compile_assert() noexcept {
		throw assert_compile_error {};
	}

	STOP_SUPPRESS_WARNINGS

	/**/

	template <typename Expr_>
		requires std::is_invocable_r_v<bool, Expr_>
	constexpr void assertrt(Expr_&& expr_) noexcept {

		if (std::is_constant_evaluated()) {
			if (not std::forward<Expr_>(expr_)()) {
				throw_compile_assert();
			}
			return;
		}

		if (not std::forward<Expr_>(expr_)()) {
			::hg::panic();
		}
	}

	template <typename Expr_>
		requires (not std::is_invocable_r_v<bool, Expr_>) &&
		std::is_invocable_v<Expr_> &&
		std::is_convertible_v<std::invoke_result_t<Expr_>, bool>
	constexpr void assertrt(Expr_&& expr_) noexcept {

		if (std::is_constant_evaluated()) {
			if (not static_cast<bool>(std::forward<Expr_>(expr_)())) {
				throw_compile_assert();
			}
			return;
		}

		if (not static_cast<bool>(std::forward<Expr_>(expr_)())) {
			::hg::panic();
		}
	}

	template <typename Expr_>
		requires (not std::is_invocable_v<Expr_>) &&
		std::is_nothrow_convertible_v<Expr_, bool>
	constexpr void assertrt(Expr_&& expr_) noexcept {

		if (std::is_constant_evaluated()) {
			if (not static_cast<bool>(std::forward<Expr_>(expr_))) {
				throw_compile_assert();
			}
			return;
		}

		if (not static_cast<bool>(std::forward<Expr_>(expr_))) {
			::hg::panic();
		}
	}

	#if defined(_DEBUG) && not defined(NDEBUG)

	template <typename Expr_>
		requires std::is_invocable_r_v<bool, Expr_>
	constexpr void assertd(Expr_&& expr_) noexcept {

		if (std::is_constant_evaluated()) {
			if (not std::forward<Expr_>(expr_)()) {
				throw_compile_assert();
			}
			return;
		}

		if (not std::forward<Expr_>(expr_)()) {
			::hg::breakpoint();
			::hg::panic();
		}
	}

	template <typename Expr_>
		requires (not std::is_invocable_r_v<bool, Expr_>) &&
		std::is_invocable_v<Expr_> &&
		std::is_convertible_v<std::invoke_result_t<Expr_>, bool>
	constexpr void assertd(Expr_&& expr_) noexcept {

		if (std::is_constant_evaluated()) {
			if (not static_cast<bool>(std::forward<Expr_>(expr_)())) {
				throw_compile_assert();
			}
			return;
		}

		if (not static_cast<bool>(std::forward<Expr_>(expr_)())) {
			::hg::breakpoint();
			::hg::panic();
		}
	}

	template <typename Expr_>
		requires (not std::is_invocable_v<Expr_>) &&
		std::is_nothrow_convertible_v<Expr_, bool>
	constexpr void assertd(Expr_&& expr_) noexcept {

		if (std::is_constant_evaluated()) {
			if (not static_cast<bool>(std::forward<Expr_>(expr_))) {
				throw_compile_assert();
			}
			return;
		}

		if (not static_cast<bool>(std::forward<Expr_>(expr_))) {
			::hg::breakpoint();
			::hg::panic();
		}
	}

	#else

	template <typename Expr_>
		requires std::is_invocable_r_v<bool, Expr_>
	constexpr void assertd(Expr_&& expr_) noexcept {
		// noop();
	}

	template <typename Expr_>
		requires (not std::is_invocable_r_v<bool, Expr_>) &&
		std::is_invocable_v<Expr_> &&
		std::is_convertible_v<std::invoke_result_t<Expr_>, bool>
	constexpr void assertd(Expr_&& expr_) noexcept {
		// noop();
	}

	template <typename Expr_>
		requires (not std::is_invocable_v<Expr_>) &&
		std::is_nothrow_convertible_v<Expr_, bool>
	constexpr void assertd(Expr_&& expr_) noexcept {
		// noop();
	}

	#endif
}
