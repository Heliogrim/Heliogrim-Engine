#pragma once

#include "TypeName.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::refl {
	template <typename Type_>
	[[nodiscard]] consteval type_id ctid() noexcept {
		constexpr auto compiled = compile_type_name<Type_>();
		return type_id { compiled.hash() };
	}
}
