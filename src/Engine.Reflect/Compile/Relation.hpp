#pragma once

#include <memory>
#include <Engine.Common/__macro.hpp>

namespace hg {
	template <typename Type_>
	struct CompileLess {
		[[nodiscard]] consteval bool operator()(const Type_& left_, const Type_& right_) const noexcept {
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
