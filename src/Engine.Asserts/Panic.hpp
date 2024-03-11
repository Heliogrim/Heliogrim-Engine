#pragma once

#include <Engine.Common/__macro.hpp>
#include <cstdlib>
#include <iostream>

#if defined(__cpp_lib_stacktrace) && (__cpp_lib_stacktrace >= 202011L) && defined(__cpp_lib_formatters) && (__cpp_lib_formatters >= 202302L)
#include <stacktrace>
#include <format>

#if _WIN32 || _WIN64
#include <windows.h>
#include <windowsx.h>
#endif

#endif

namespace hg {
	namespace {
		#if defined(_WIN32) && defined(_MSC_VER)

		template <typename... Args_>
		using __panic_stacktrace_stream = decltype([](Args_&&... args_) {
			((std::cerr << std::forward<Args_>(args_) << '\n'), ...);
			std::cerr << std::flush;

			(OutputDebugString(std::format("{}", std::forward<Args_>(args_)).c_str()), ...);
		});

		#else

		template <typename... Args_>
		using __panic_stacktrace_stream = decltype([](Args_&&... args_) {
			((std::cerr << std::forward<Args_>(args_) << '\n'), ...);
			std::cerr << std::flush;
		});

		#endif
	}

	#if defined(__cpp_lib_stacktrace) && (__cpp_lib_stacktrace >= 202011L) && defined(__cpp_lib_formatters) && (__cpp_lib_formatters >= 202302L)

	template <template <typename...> typename ErrorStream_ = __panic_stacktrace_stream>
	void panic_stacktrace() noexcept {
		using stream_type = ErrorStream_<decltype(std::stacktrace::current())>;
		(stream_type {})(std::stacktrace::current());
	}

	#else

	template <template <typename...> typename ErrorStream_ = __panic_stacktrace_stream>
	void panic_stacktrace() noexcept {
		// noop();
	}

	#endif

	[[noreturn]] FORCE_INLINE inline void panic() noexcept {
		::hg::panic_stacktrace();
		::std::abort();
	}
}
