#pragma once

#include <cstddef>
#include <span>
#include <type_traits>

#define ACCESS_LAYOUT template <typename> friend class ::hg::engine::serialization::access::Layout;
#define ACCESS_STRUCTURE template <class, class> friend class ::hg::engine::serialization::access::Structure;

namespace hg::engine::serialization::access {
	template <class Type_>
	class Layout;

	template <class Type_, class Scheme_>
	class Structure;

	/**
	 * Leaves a layout may copy as raw bytes: non-bool integers, float, double and arrays of these. Every bit pattern of
	 * such a leaf is a valid value; bool, enums, pointers and handles never qualify.
	 */
	template <typename Type_>
	concept raw_copyable_leaf =
		(std::is_integral_v<std::remove_cv_t<std::remove_all_extents_t<Type_>>> &&
			not std::is_same_v<std::remove_cv_t<std::remove_all_extents_t<Type_>>, bool>) ||
		std::is_same_v<std::remove_cv_t<std::remove_all_extents_t<Type_>>, float> ||
		std::is_same_v<std::remove_cv_t<std::remove_all_extents_t<Type_>>, double>;

	enum class LayoutFieldKind : unsigned char {
		// Copied verbatim.
		eRaw,
		// One byte whose bits outside `mask` must be zero (bool, bitfield flags).
		eMask
	};

	struct LayoutField {
		LayoutFieldKind kind;
		unsigned long long offset;
		unsigned long long size;
		unsigned char mask;
	};

	template <typename Leaf_>
	[[nodiscard]] consteval LayoutField raw_field(const unsigned long long offset_, const unsigned long long memberSize_) {
		static_assert(raw_copyable_leaf<Leaf_>, "Raw layout fields are non-bool integers, float, double or arrays of these.");
		if (sizeof(Leaf_) != memberSize_) {
			throw "Raw layout field leaf does not match the member size.";
		}
		return LayoutField { LayoutFieldKind::eRaw, offset_, sizeof(Leaf_), 0u };
	}

	[[nodiscard]] consteval LayoutField mask_field(
		const unsigned long long offset_,
		const unsigned long long memberSize_,
		const unsigned char mask_
	) {
		if (memberSize_ == 0uLL || mask_ == 0u) {
			throw "Masked layout fields need a non-empty member and mask.";
		}
		return LayoutField { LayoutFieldKind::eMask, offset_, 1uLL, mask_ };
	}

	/* `Layout<T>::fields` lists the data a layout streams; the copy plan in ArchetypeLayout.inl.hpp derives from it. */
	template <typename Type_>
	concept layout_fields = requires {
		std::span<const LayoutField> { Layout<Type_>::fields };
	};
}

// offsetof on non-standard-layout components is conditionally supported; clang folds it to a constant.
// Fields are byte offsets rather than member pointers: the copy plan needs constant byte ranges, which member pointers do not give (P28).
#define HG_LAYOUT_OFFSETOF(Type_, Member_) \
	_Pragma("clang diagnostic push") \
	_Pragma("clang diagnostic ignored \"-Winvalid-offsetof\"") \
	offsetof(Type_, Member_) \
	_Pragma("clang diagnostic pop")

#define HG_LAYOUT_RAW(Type_, Member_, Leaf_) \
	::hg::engine::serialization::access::raw_field<Leaf_>(HG_LAYOUT_OFFSETOF(Type_, Member_), sizeof(Type_::Member_))

// Byte `Member_` (bool or a one-byte bitfield struct, low byte only) with the bits of `Mask_` in use.
#define HG_LAYOUT_MASK(Type_, Member_, Mask_) \
	::hg::engine::serialization::access::mask_field(HG_LAYOUT_OFFSETOF(Type_, Member_), sizeof(Type_::Member_), Mask_)
