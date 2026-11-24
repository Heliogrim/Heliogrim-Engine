#pragma once

#include <concepts>
#include <cstddef>
#include <ranges>
#include <span>
#include <type_traits>

#include "../Wrapper.hpp"

namespace hg {
	template <std::integral Type_>
	struct IndexSlice {
		Type_ begin;
		Type_ end;
	};

	/**
	 * Part index_ of total_ split into count_ parts: [begin, end). The first total_ % count_ parts hold one more element;
	 *	parts past the end of a short total_ are empty. Requires count_ > 0 and index_ < count_.
	 *	Type_ comes from total_ alone, so `sliceOf(items.size(), index_, count_)` with u16 parts works.
	 */
	template <std::integral Type_>
	[[nodiscard]] constexpr IndexSlice<Type_> sliceOf(
		const Type_ total_,
		const std::type_identity_t<Type_> index_,
		const std::type_identity_t<Type_> count_
	) noexcept {
		const Type_ base = total_ / count_;
		const Type_ rest = total_ % count_;
		// static_cast: arithmetic on types below int promotes to int
		const auto begin = static_cast<Type_>(index_ * base + (index_ < rest ? index_ : rest));
		return IndexSlice<Type_> { .begin = begin, .end = static_cast<Type_>(begin + base + (index_ < rest ? 1 : 0)) };
	}

	/**
	 * Part index_ of range_ split into count_ parts, as sliceOf above: a std::span for a contiguous range, a subrange otherwise.
	 *	Borrows range_; takes lvalues only so the result cannot outlive a temporary.
	 */
	template <std::ranges::random_access_range Range_, std::integral Index_> requires std::ranges::sized_range<Range_>
	[[nodiscard]] constexpr auto sliceOf(ref<Range_> range_, const Index_ index_, const Index_ count_) {
		const auto [begin, end] = sliceOf<std::size_t>(std::ranges::size(range_), index_, count_);
		if constexpr (std::ranges::contiguous_range<Range_>) {
			return std::span { std::ranges::data(range_) + begin, end - begin };
		} else {
			using diff = std::ranges::range_difference_t<Range_>;
			const auto first = std::ranges::begin(range_);
			return std::ranges::subrange { first + static_cast<diff>(begin), first + static_cast<diff>(end) };
		}
	}
}
