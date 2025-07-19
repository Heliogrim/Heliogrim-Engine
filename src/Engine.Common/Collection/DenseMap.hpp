#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/__macro.hpp>

START_SUPPRESS_WARNINGS
#include <ankerl/unordered_dense.h>
STOP_SUPPRESS_WARNINGS

namespace hg {
	template <
		class Key_,
		class Type_,
		class Hash_ = ankerl::unordered_dense::v4_5_0::hash<Key_>,
		class Equal_ = std::equal_to<Key_>,
		class Alloc_ = std::allocator<std::pair<Key_, Type_>>>
	using DenseMap = ankerl::unordered_dense::map<Key_, Type_, Hash_, Equal_, Alloc_>;
}
