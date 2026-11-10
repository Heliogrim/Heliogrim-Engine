#pragma once

#include "Range.hpp"

namespace hg {
	struct CopyToFn final {
		template <typename Iter_, typename Sentinel_, typename OutIter_>
		constexpr OutIter_ operator()(Iter_ first_, const Sentinel_& last_, OutIter_ out_) const {
			for (; first_ != last_; ++first_, ++out_) {
				*out_ = *first_;
			}
			return out_;
		}

		template <IterableRange Range_, typename OutIter_>
		constexpr OutIter_ operator()(Range_&& range_, OutIter_ out_) const {
			return (*this)(range_.begin(), range_.end(), out_);
		}
	};

	inline constexpr CopyToFn CopyTo {};

	struct TransformToFn final {
		template <typename Iter_, typename Sentinel_, typename OutIter_, typename Fn_>
		constexpr OutIter_ operator()(Iter_ first_, const Sentinel_& last_, OutIter_ out_, Fn_&& fn_) const {
			for (; first_ != last_; ++first_, ++out_) {
				*out_ = fn_(*first_);
			}
			return out_;
		}

		template <IterableRange Range_, typename OutIter_, typename Fn_>
		constexpr OutIter_ operator()(Range_&& range_, OutIter_ out_, Fn_&& fn_) const {
			return (*this)(range_.begin(), range_.end(), out_, fn_);
		}
	};

	inline constexpr TransformToFn TransformTo {};

	/**
	 * Stable merge of two sorted ranges: on equal elements the first range wins.
	 */
	struct MergeFn final {
		template <typename LeftIter_, typename RightIter_, typename OutIter_, typename Less_ = LessFn>
		constexpr OutIter_ operator()(
			LeftIter_ left_,
			const LeftIter_& leftLast_,
			RightIter_ right_,
			const RightIter_& rightLast_,
			OutIter_ out_,
			Less_ less_ = {}
		) const {
			for (; left_ != leftLast_ && right_ != rightLast_; ++out_) {
				if (less_(*right_, *left_)) {
					*out_ = *right_;
					++right_;
				} else {
					*out_ = *left_;
					++left_;
				}
			}
			out_ = CopyTo(left_, leftLast_, out_);
			return CopyTo(right_, rightLast_, out_);
		}
	};

	inline constexpr MergeFn Merge {};
}
