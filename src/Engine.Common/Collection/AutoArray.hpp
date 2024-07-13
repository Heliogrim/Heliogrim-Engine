#pragma once

#include <bit>
#include <span>

#include "Vector.hpp"
#include "../Wrapper.hpp"

namespace hg {
	// TODO: Custom Implementation

	template <typename Type_, typename Allocator_ = std::allocator<Type_>>
	class AutoArray :
		public Vector<Type_, Allocator_> {
	public:
		using Vector<Type_, Allocator_>::Vector;

	public:
		[[nodiscard]] std::span<_::byte> getByteSpan() noexcept requires (std::is_trivial_v<Type_>) {
			return std::span { std::bit_cast<ptr<_::byte>>(this->data()), this->size() * sizeof(Type_) };
		}
	};
}
