#pragma once

#include <Engine.Common/String.hpp>
#include <Engine.Common/__macro.hpp>

#include "String.hpp"

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
