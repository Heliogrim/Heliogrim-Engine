#pragma once

#include <bit>
#include <span>
#include <vector>

#include "../Types.hpp"
#include "../Wrapper.hpp"
#include "../__macro.hpp"

namespace hg {
	template <typename Type_, typename Allocator_ = std::allocator<Type_>>
	class Vector :
		public std::vector<Type_, Allocator_> {
	public:
		using this_type = Vector<Type_, Allocator_>;
		using base_type = std::vector<Type_, Allocator_>;

	public:
		using base_type::vector;

		constexpr Vector(mref<base_type> vector_) noexcept :// NOLINT(*-explicit-constructor)
			base_type(std::move(vector_)) {}

	public:
		[[nodiscard]] std::span<_::byte> getByteSpan() noexcept requires (std::is_trivial_v<Type_>) {
			return std::span { std::bit_cast<ptr<_::byte>>(this->data()), this->size() * sizeof(Type_) };
		}

	public:
		template <std::ranges::range Range_>
		constexpr void insert_range(typename base_type::const_iterator where_, Range_&& range_) {
			#if defined(__cpp_lib_containers_ranges) && __cpp_lib_containers_ranges >= 202202L
			base_type::insert_range(std::forward<decltype(where_)>(where_), std::forward<Range_>(range_));
			#else
			base_type::insert(std::forward<decltype(where_)>(where_), range_.begin(), range_.end());
			#endif
		}

		template <std::ranges::range Range_>
		constexpr void append_range(Range_&& range_) {
			#if defined(__cpp_lib_containers_ranges) && __cpp_lib_containers_ranges >= 202202L
			base_type::append_range(std::forward<Range_>(range_));
			#else
			base_type::insert(base_type::end(), range_.begin(), range_.end());
			#endif
		}
	};
}
