#pragma once

#include <stdexcept>
#include <type_traits>
#include <Engine.Common/__macro.hpp>

#include "Breakpoint.hpp"
#include "Panic.hpp"

namespace hg {
	template <typename Expr_>
		requires std::is_invocable_r_v<bool, Expr_>
	void assertrt(Expr_&& expr_) noexcept {
		if (not std::forward<Expr_>(expr_)()) {
			::hg::panic();
		}
	}

	template <typename Expr_>
		requires (not std::is_invocable_r_v<bool, Expr_>) &&
		std::is_invocable_v<Expr_> &&
		std::is_convertible_v<std::invoke_result_t<Expr_>, bool>
	void assertrt(Expr_&& expr_) noexcept {
		if (not static_cast<bool>(std::forward<Expr_>(expr_)())) {
			::hg::panic();
		}
	}

	template <typename Expr_>
		requires (not std::is_invocable_v<Expr_>) &&
		std::is_nothrow_convertible_v<Expr_, bool>
	void assertrt(Expr_&& expr_) noexcept {
		if (not static_cast<bool>(std::forward<Expr_>(expr_))) {
			::hg::panic();
		}
	}

	#if defined(_DEBUG) && not defined(NDEBUG)

	template <typename Expr_>
		requires std::is_invocable_r_v<bool, Expr_>
	void assertd(Expr_&& expr_) noexcept {
		if (not std::forward<Expr_>(expr_)()) {
			::hg::breakpoint();
			::hg::panic();
		}
	}

	template <typename Expr_>
		requires (not std::is_invocable_r_v<bool, Expr_>) &&
		std::is_invocable_v<Expr_> &&
		std::is_convertible_v<std::invoke_result_t<Expr_>, bool>
	void assertd(Expr_&& expr_) noexcept {
		if (not static_cast<bool>(std::forward<Expr_>(expr_)())) {
			::hg::breakpoint();
			::hg::panic();
		}
	}

	template <typename Expr_>
		requires (not std::is_invocable_v<Expr_>) &&
		std::is_nothrow_convertible_v<Expr_, bool>
	void assertd(Expr_&& expr_) noexcept {
		if (not static_cast<bool>(std::forward<Expr_>(expr_))) {
			::hg::breakpoint();
			::hg::panic();
		}
	}

	#else

	template <typename Expr_>
		requires std::is_invocable_r_v<bool, Expr_>
	void assertd(Expr_&& expr_) noexcept {
		// noop();
	}

	template <typename Expr_>
		requires (not std::is_invocable_r_v<bool, Expr_>) &&
		std::is_invocable_v<Expr_> &&
		std::is_convertible_v<std::invoke_result_t<Expr_>, bool>
	void assertd(Expr_&& expr_) noexcept {
		// noop();
	}

	template <typename Expr_>
		requires (not std::is_invocable_v<Expr_>) &&
		std::is_nothrow_convertible_v<Expr_, bool>
	void assertd(Expr_&& expr_) noexcept {
		// noop();
	}

	#endif
}
