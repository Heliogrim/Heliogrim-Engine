#pragma once

#include "__macro.hpp"
#include "Meta/Reference.hpp"

namespace hg {
	template <typename Type_>
	[[nodiscard]] macro_attr_intr constexpr Type_&& forward(::hg::meta::remove_reference_t<Type_>& ref_) noexcept {
		return static_cast<Type_&&>(ref_);
	}

	template <typename Type_>
	[[nodiscard]] macro_attr_intr constexpr Type_&& forward(::hg::meta::remove_reference_t<Type_>&& ref_) noexcept {
		return static_cast<Type_&&>(ref_);
	}
}
