#pragma once

#include <Engine.Common/__macro.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import std;
#endif

namespace hg {
	template <typename Type_>
	struct CompileLess {
		[[nodiscard]] constexpr bool operator()(const Type_& left_, const Type_& right_) const noexcept {
			return left_ < right_;
		}
	};

	template <typename Type_>
	struct CompileSwap {
		consteval void operator()(Type_& left_, Type_& right_) const noexcept {
			constexpr Type_ tmp { std::move(left_) };
			left_ = std::move(right_);
			right_ = std::move(tmp);
		}
	};
}
