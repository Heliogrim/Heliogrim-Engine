#pragma once

#include <stdexcept>

#include "Panic.hpp"

namespace hg {
	template <typename ErrorType_>
	struct ErrorPanicFn;

	template <typename ErrorType_> requires std::is_constructible_v<ErrorType_>
	struct ErrorPanicFn<ErrorType_> {
		[[noreturn]] constexpr void operator()() const noexcept {
			::hg::panic();
		}
	};

	template <typename ErrorType_> requires (not std::is_constructible_v<ErrorType_>)
	struct ErrorPanicFn<ErrorType_> {
		static_assert(std::is_constructible_v<ErrorType_>);
	};

	/**/

	class todo_runtime_error :
		public std::runtime_error {
	public:
		using std::runtime_error::runtime_error;

		todo_runtime_error() noexcept:
			std::runtime_error("Not Implemented!") {}
	};

	/**/

	using TodoPanicFn = ErrorPanicFn<::hg::todo_runtime_error>;
	static constexpr TodoPanicFn todo_panic {};
}
