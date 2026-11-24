#pragma once

#define ACCESS_LAYOUT template <typename> friend class ::hg::engine::serialization::access::Layout;
#define ACCESS_STRUCTURE template <class, class> friend class ::hg::engine::serialization::access::Structure;

namespace hg::engine::serialization::access {
	template <class Type_>
	class Layout;

	template <class Type_, class Scheme_>
	class Structure;
}

// offsetof (spelled __builtin_offsetof: the macro does not cross `import std`) on non-standard-layout components is conditionally supported; clang folds it to a constant.
// Fields are byte offsets rather than member pointers: the copy plan needs constant byte ranges, which member pointers do not give (P28).
#define HG_LAYOUT_OFFSETOF(Type_, Member_) \
	_Pragma("clang diagnostic push") \
	_Pragma("clang diagnostic ignored \"-Winvalid-offsetof\"") \
	__builtin_offsetof(Type_, Member_) \
	_Pragma("clang diagnostic pop")

#define HG_LAYOUT_RAW(Type_, Member_, Leaf_) \
	::hg::engine::serialization::access::raw_field<Leaf_>(HG_LAYOUT_OFFSETOF(Type_, Member_), sizeof(Type_::Member_), #Member_)

// Byte `Member_` (bool or a one-byte bitfield struct, low byte only) with the bits of `Mask_` in use.
#define HG_LAYOUT_MASK(Type_, Member_, Mask_) \
	::hg::engine::serialization::access::mask_field(HG_LAYOUT_OFFSETOF(Type_, Member_), sizeof(Type_::Member_), Mask_, #Member_)
