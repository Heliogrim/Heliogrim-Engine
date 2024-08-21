#pragma once

#include <Engine.Common/__macro.hpp>

namespace hg {
	#if defined(_DEBUG) && not defined(NDEBUG)

	inline void breakpoint() {
		#if defined(ENV_WIN)
		__debugbreak();
		#elif defined(__clang__)
		__builtin_debugtrap();
		#elif defined(_MSC_VER)
		_asm {int 3}
		#elif defined(__has_builtin) && __has_builtin(__builtin_debugtrap)
		__builtin_debugtrap();
		#elif (defined(__GNUC__) || defined(__clang__))
		asm("int3");
		#elif ENV_INST_ARM
		asm("brk #0x1");
		#else
		static_assert(false, "Failed to determine debugbreak function.");
		#endif
	}

	#else

	inline void breakpoint() {
		// noop();
	}

	#endif
}
