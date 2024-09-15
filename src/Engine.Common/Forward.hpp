#pragma once

#include "Meta/Reference.hpp"

namespace hg {
	template <typename Type_>
	[[nodiscard, msvc::intrinsic]] constexpr Type_&& forward(::hg::meta::remove_reference_t<Type_>& ref_) noexcept {
		return static_cast<Type_&&>(ref_);
	}

	template <typename Type_>
	[[nodiscard, msvc::intrinsic]] constexpr Type_&& forward(::hg::meta::remove_reference_t<Type_>&& ref_) noexcept {
		return static_cast<Type_&&>(ref_);
	}
}
