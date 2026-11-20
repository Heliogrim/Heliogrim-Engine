#pragma once

#include <Engine.Common/__macro.hpp>
#include <Engine.Common/Wrapper.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import ankerl.unordered_dense;
#endif

START_SUPPRESS_WARNINGS
STOP_SUPPRESS_WARNINGS

namespace hg {
	template <
		class Key_,
		class Type_,
		class Hash_ = ankerl::unordered_dense::v5_3_3::hash<Key_>,
		class Equal_ = std::equal_to<Key_>,
		class Alloc_ = std::allocator<std::pair<Key_, Type_>>>
	using DenseMap = ankerl::unordered_dense::map<Key_, Type_, Hash_, Equal_, Alloc_>;
}
