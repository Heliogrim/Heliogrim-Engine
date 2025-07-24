#pragma once

#include <algorithm>
#include <Engine.Common/__macro.hpp>
#include <Engine.Common/Collection/Vector.hpp>

namespace hg {
	namespace {
		template <class Iter_, class OutIter_, typename Pred_>
		consteval void __ct_sort(Iter_ first_, Iter_ last_, OutIter_ out_, Pred_ pred_) {

			const auto distance = std::distance(first_, last_);
			if (distance < 2) {
				std::copy(first_, last_, out_);
				return;
			}

			const auto src_part = std::next(first_, distance / 2);
			const auto dst_part = std::next(out_, distance / 2);

			__ct_sort(first_, src_part, out_, pred_);
			__ct_sort(src_part, last_, dst_part, pred_);

			std::merge(out_, dst_part, dst_part, std::next(out_, distance), first_, pred_);
			std::copy(first_, last_, out_);
		}
	}

	template <typename Iter_, typename Pred_ = std::less<std::remove_cvref_t<decltype(*std::declval<Iter_>())>>>
	consteval void compile_sort(Iter_ first_, Iter_ last_, Pred_ pred_ = {}) {
		Vector<typename Iter_::value_type> tmp(first_, last_);
		__ct_sort(first_, last_, tmp.begin(), pred_);
	}
}
