#pragma once

namespace hg {
	/**
	 * Anything with member begin() and end(); avoids pulling <ranges> into every header that iterates.
	 */
	template <typename Range_>
	concept IterableRange = requires(Range_& range_) {
		range_.begin();
		range_.end();
	};

	struct IdentityFn final {
		template <typename Type_>
		[[nodiscard]] constexpr Type_&& operator()(Type_&& value_) const noexcept {
			return static_cast<Type_&&>(value_);
		}
	};

	struct LessFn final {
		template <typename Left_, typename Right_>
		[[nodiscard]] constexpr bool operator()(const Left_& left_, const Right_& right_) const {
			return left_ < right_;
		}
	};
}
