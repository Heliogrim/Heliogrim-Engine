#pragma once

#include "Vector.hpp"

namespace hg {
	// TODO: Custom Implementation

	template <typename Type_, typename Allocator_ = std::allocator<Type_>>
	class AutoArray :
		public Vector<Type_, Allocator_> {
	public:
		using Vector<Type_, Allocator_>::Vector;
	};
}
