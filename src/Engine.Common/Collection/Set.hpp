#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import ankerl.unordered_dense;
import std;
#endif

namespace hg {
	template <
		class Type_,
		class Hash_ = ankerl::unordered_dense::v5_3_3::hash<Type_>,
		class Equal_ = std::equal_to<Type_>,
		class Allocator_ = std::allocator<Type_>>
	using CompactSet = ankerl::unordered_dense::set<Type_, Hash_, Equal_, Allocator_>;

	template <
		class Type_,
		class Hash_ = std::hash<Type_>,
		class Equal_ = std::equal_to<Type_>,
		class Allocator_ = std::allocator<Type_>>
	using Set = std::unordered_set<Type_, Hash_, Equal_, Allocator_>;
}
