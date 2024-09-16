#pragma once

#include "__macro.hpp"
#include "Meta/Reference.hpp"

namespace hg {
	template <typename Type_>
	[[nodiscard]] macro_attr_intr constexpr ::hg::meta::remove_reference_t<Type_>&& move(Type_&& value_) noexcept {
		return static_cast<::hg::meta::remove_reference_t<Type_>&&>(value_);
	}
}
