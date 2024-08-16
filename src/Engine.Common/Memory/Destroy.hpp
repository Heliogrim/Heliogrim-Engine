#pragma once

#include <type_traits>

#include "Address.hpp"
#include "../__macro.hpp"

namespace hg {
	/* Note: This is a straight copy from the msvc xmemory header... */

	template <class Allocator_, typename ValueType_>
	concept AllocIsDefault = std::is_same_v<std::allocator<ValueType_>, Allocator_>;

	template <class Allocator_, typename PointerType_>
	concept AllocHasDestroy = requires(Allocator_& alloc_) {
		{ alloc_.destroy(std::declval<PointerType_>()) };
	};

	template <class Allocator_>
	constexpr void destroy_range(
		typename std::allocator_traits<Allocator_>::pointer first_,
		const typename std::allocator_traits<Allocator_>::pointer last_,
		Allocator_& alloc_
	) noexcept {
		using value_type = typename Allocator_::value_type;
		if constexpr (not(
			std::is_trivially_destructible_v<value_type> &&
			(AllocIsDefault<Allocator_, value_type> || not AllocHasDestroy<Allocator_, value_type*>)
		)) {
			for (; first_ != last_; GUARD_UNCHECKED_LLVL(++first_)) {
				std::allocator_traits<Allocator_>::destroy(alloc_, ::hg::unfancy(first_));
			}
		}
	}
}
