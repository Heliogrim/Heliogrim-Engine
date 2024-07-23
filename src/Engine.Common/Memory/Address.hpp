#pragma once

#include <memory>

namespace hg {
	template <typename FancyPointer_>
	[[nodiscard]] constexpr auto unfancy(FancyPointer_ pointer_) noexcept {
		return std::addressof(*pointer_);
	}

	template <typename PlainPointer_>
	[[nodiscard]] constexpr auto unfancy(PlainPointer_* pointer_) noexcept {
		return pointer_;
	}

	template <typename FancyPointer_>
	[[nodiscard]] constexpr auto unfancyNullable(FancyPointer_ pointer_) noexcept {
		return pointer_ ? std::addressof(*pointer_) : nullptr;
	}

	template <typename PlainPointer_>
	[[nodiscard]] constexpr auto unfancyNullable(PlainPointer_* pointer_) noexcept {
		return pointer_;
	}
}
