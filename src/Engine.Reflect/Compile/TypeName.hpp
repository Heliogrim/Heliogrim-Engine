#pragma once

#include <Engine.Common/__macro.hpp>

#include "String.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::refl {
	namespace {
		START_SUPPRESS_WARNINGS

		template <typename Type_>
		consteval CompileString compile_type_name() {
			return { __FUNCSIG__ };
		}

		STOP_SUPPRESS_WARNINGS
	}

	template <typename Type_>
	[[nodiscard]] consteval StringView ctname() noexcept {
		constexpr auto compiled = compile_type_name<Type_>();
		return StringView { compiled.begin(), compiled.end() };
	}
}
