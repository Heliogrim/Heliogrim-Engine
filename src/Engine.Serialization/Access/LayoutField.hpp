#pragma once

#include "__fwd.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import std;
#endif

namespace hg::engine::serialization::access {
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
		// Member name; with kind, size and mask it identifies the field in the layout hash, the offset does not.
		const char* name;
	};

	template <typename Leaf_>
	[[nodiscard]] consteval LayoutField raw_field(
		const unsigned long long offset_,
		const unsigned long long memberSize_,
		const char* const name_
	) {
		static_assert(raw_copyable_leaf<Leaf_>, "Raw layout fields are non-bool integers, float, double or arrays of these.");
		if (sizeof(Leaf_) != memberSize_) {
			throw "Raw layout field leaf does not match the member size.";
		}

		return LayoutField { LayoutFieldKind::eRaw, offset_, sizeof(Leaf_), 0u, name_ };
	}

	[[nodiscard]] consteval LayoutField mask_field(
		const unsigned long long offset_,
		const unsigned long long memberSize_,
		const unsigned char mask_,
		const char* const name_
	) {
		if (memberSize_ == 0uLL || mask_ == 0u) {
			throw "Masked layout fields need a non-empty member and mask.";
		}

		return LayoutField { LayoutFieldKind::eMask, offset_, 1uLL, mask_, name_ };
	}

	/* `Layout<T>::fields` lists the data a layout streams; the copy plan in ArchetypeLayout.inl.hpp derives from it. */
	template <typename Type_>
	concept layout_fields = requires {
		std::span<const LayoutField> { Layout<Type_>::fields };
	};
}
