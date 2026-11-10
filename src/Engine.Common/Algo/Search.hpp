#pragma once

#include <cstddef>

#include "Range.hpp"

namespace hg {
	struct FindIfFn final {
		template <typename Iter_, typename Sentinel_, typename Pred_>
		[[nodiscard]] constexpr Iter_ operator()(Iter_ first_, const Sentinel_& last_, Pred_&& pred_) const {
			for (; first_ != last_; ++first_) {
				if (pred_(*first_)) {
					return first_;
				}
			}
			return first_;
		}

		template <IterableRange Range_, typename Pred_>
		[[nodiscard]] constexpr auto operator()(Range_& range_, Pred_&& pred_) const {
			return (*this)(range_.begin(), range_.end(), pred_);
		}
	};

	inline constexpr FindIfFn FindIf {};

	struct CountOfFn final {
		template <typename Iter_, typename Sentinel_, typename Value_>
		[[nodiscard]] constexpr std::size_t operator()(Iter_ first_, const Sentinel_& last_, const Value_& value_) const {
			std::size_t count = 0;
			for (; first_ != last_; ++first_) {
				if (*first_ == value_) {
					++count;
				}
			}
			return count;
		}

		template <IterableRange Range_, typename Value_>
		[[nodiscard]] constexpr std::size_t operator()(Range_&& range_, const Value_& value_) const {
			return (*this)(range_.begin(), range_.end(), value_);
		}
	};

	inline constexpr CountOfFn CountOf {};

	/**
	 * First element in a sorted random-access range whose projection is not less than value_.
	 */
	struct LowerBoundFn final {
		template <typename Iter_, typename Value_, typename Less_ = LessFn, typename Proj_ = IdentityFn>
			requires (not IterableRange<Iter_>)
		[[nodiscard]] constexpr Iter_ operator()(
			Iter_ first_,
			const Iter_& last_,
			const Value_& value_,
			Less_ less_ = {},
			Proj_ proj_ = {}
		) const {
			auto count = last_ - first_;
			while (count > 0) {
				const auto step = count / 2;
				const auto mid = first_ + step;
				if (less_(proj_(*mid), value_)) {
					first_ = mid + 1;
					count -= step + 1;
				} else {
					count = step;
				}
			}
			return first_;
		}

		template <IterableRange Range_, typename Value_, typename Less_ = LessFn, typename Proj_ = IdentityFn>
		[[nodiscard]] constexpr auto operator()(Range_& range_, const Value_& value_, Less_ less_ = {}, Proj_ proj_ = {}) const {
			return (*this)(range_.begin(), range_.end(), value_, less_, proj_);
		}
	};

	inline constexpr LowerBoundFn LowerBound {};
}
