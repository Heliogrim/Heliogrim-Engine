#pragma once

#include <ranges>

#include "../Optional.hpp"

namespace hg {
	template <typename Type_>
	struct FirstOfFn final {
		template <::std::ranges::range Rng_, typename Selector_> requires IsOpt<Type_>
		[[nodiscard]] constexpr decltype(auto) operator()(Rng_&& range_, Selector_&& selector_) const noexcept {
			for (const auto& el : range_) {
				if (auto maybe = selector_(el); maybe != None) {
					return maybe;
				}
			}

			return decltype(selector_(*range_.begin())) { None };
		}
	};

	template <typename Type_>
	constexpr inline FirstOfFn<Type_> FirstOf {};
}
