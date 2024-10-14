#pragma once

#include <Engine.Common/Meta/TypeId.hpp>

#include "TypeName.hpp"

namespace hg::refl {
	template <typename Type_>
	[[nodiscard]] consteval type_id ctid() noexcept {
		const auto compiled = compile_type_name<Type_>();
		return type_id { compiled.hash() };
	}
}
