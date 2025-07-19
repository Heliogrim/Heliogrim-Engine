#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/__macro.hpp>

START_SUPPRESS_WARNINGS
#include <ankerl/unordered_dense.h>
STOP_SUPPRESS_WARNINGS

namespace hg {
	template <
		class Type_,
		class Hash_ = ankerl::unordered_dense::v4_5_0::hash<Type_>,
		class Equal_ = std::equal_to<Type_>,
		class Allocator_ = std::allocator<Type_>>
	using DenseSet = ankerl::unordered_dense::set<Type_, Hash_, Equal_, Allocator_>;
}
