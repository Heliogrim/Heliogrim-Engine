#pragma once

#include "Meta/Reference.hpp"

namespace hg {
	template <typename Type_>
	[[nodiscard, msvc::intrinsic]] constexpr ::hg::meta::remove_reference_t<Type_>&& move(Type_&& value_) noexcept {
		return static_cast<::hg::meta::remove_reference_t<Type_>&&>(value_);
	}
}
