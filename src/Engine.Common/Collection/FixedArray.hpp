#pragma once

#include "Array.hpp"

namespace hg {
	template <class Type_, size_t Size_>
	using FixedArray = Array<Type_, Size_>;
}