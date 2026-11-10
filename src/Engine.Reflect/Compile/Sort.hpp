#pragma once

#include <Engine.Common/__macro.hpp>
#include <Engine.Common/Algo/Copy.hpp>
#include <Engine.Common/Collection/Array.hpp>

namespace hg {
	namespace {
		template <class Iter_, class OutIter_, typename Pred_>
		consteval void __ct_sort(Iter_ first_, Iter_ last_, OutIter_ out_, const Pred_& pred_) {

			const auto distance = last_ - first_;
			if (distance < 2) {
				CopyTo(first_, last_, out_);
				return;
			}

			const auto src_part = first_ + distance / 2;
			const auto dst_part = out_ + distance / 2;

			__ct_sort(first_, src_part, out_, pred_);
			__ct_sort(src_part, last_, dst_part, pred_);

			Merge(out_, dst_part, dst_part, out_ + distance, first_, pred_);
			CopyTo(first_, last_, out_);
		}
	}

	template <std::size_t Size_, typename Iter_, typename Pred_ = LessFn>
	consteval void compile_sort(Iter_ first_, Iter_ last_, Pred_ pred_ = {}) {
		Array<typename Iter_::value_type, Size_> scratchpad {};
		CopyTo(first_, last_, scratchpad.begin());
		__ct_sort(first_, last_, scratchpad.begin(), pred_);
	}
}
