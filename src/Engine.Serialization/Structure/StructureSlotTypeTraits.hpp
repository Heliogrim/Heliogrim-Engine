#pragma once

#include <Engine.Common/String.hpp>

#include "StructureSlotType.hpp"

namespace hg::engine::serialization {
	struct StructureSlotTypeTrait {
		[[nodiscard]] static string_view canonical(StructureSlotType type_) noexcept;

		[[nodiscard]] static s64 const_size(StructureSlotType type_) noexcept;
	};

	template <StructureSlotType Value_>
	struct StructureSlotTypeTraits;

	/**/

	template <>
	struct StructureSlotTypeTraits<StructureSlotType::eUndefined> {
		constexpr static string_view canonical = "Undefined"sv;

		constexpr static s64 const_size = -1LL;
	};

	template <>
	struct StructureSlotTypeTraits<StructureSlotType::eU8> {
		constexpr static string_view canonical = "U8"sv;

		constexpr static s64 const_size = 1LL;
	};

	template <>
	struct StructureSlotTypeTraits<StructureSlotType::eU16> {
		constexpr static string_view canonical = "U16"sv;

		constexpr static s64 const_size = 2LL;
	};

	template <>
	struct StructureSlotTypeTraits<StructureSlotType::eU32> {
		constexpr static string_view canonical = "U32"sv;

		constexpr static s64 const_size = 4LL;
	};

	template <>
	struct StructureSlotTypeTraits<StructureSlotType::eU64> {
		constexpr static string_view canonical = "U64"sv;

		constexpr static s64 const_size = 8LL;
	};

	template <>
	struct StructureSlotTypeTraits<StructureSlotType::eBool> {
		constexpr static string_view canonical = "Bool"sv;

		constexpr static s64 const_size = StructureSlotTypeTraits<StructureSlotType::eU8>::const_size;
	};

	template <>
	struct StructureSlotTypeTraits<StructureSlotType::eS8> {
		constexpr static string_view canonical = "S8"sv;

		constexpr static s64 const_size = StructureSlotTypeTraits<StructureSlotType::eU8>::const_size;
	};

	template <>
	struct StructureSlotTypeTraits<StructureSlotType::eS16> {
		constexpr static string_view canonical = "S16"sv;

		constexpr static s64 const_size = StructureSlotTypeTraits<StructureSlotType::eU16>::const_size;
	};

	template <>
	struct StructureSlotTypeTraits<StructureSlotType::eS32> {
		constexpr static string_view canonical = "S32"sv;

		constexpr static s64 const_size = StructureSlotTypeTraits<StructureSlotType::eU32>::const_size;
	};

	template <>
	struct StructureSlotTypeTraits<StructureSlotType::eS64> {
		constexpr static string_view canonical = "S64"sv;

		constexpr static s64 const_size = StructureSlotTypeTraits<StructureSlotType::eU64>::const_size;
	};

	template <>
	struct StructureSlotTypeTraits<StructureSlotType::eFloat> {
		constexpr static string_view canonical = "Float"sv;

		constexpr static s64 const_size = 4LL;
	};

	template <>
	struct StructureSlotTypeTraits<StructureSlotType::eDouble> {
		constexpr static string_view canonical = "Double"sv;

		constexpr static s64 const_size = 8LL;
	};

	template <>
	struct StructureSlotTypeTraits<StructureSlotType::eString> {
		constexpr static string_view canonical = "String"sv;

		constexpr static s64 const_size = 0LL;
	};

	template <>
	struct StructureSlotTypeTraits<StructureSlotType::eSlice> {
		constexpr static string_view canonical = "Slice"sv;

		constexpr static s64 const_size = 0LL;
	};

	template <>
	struct StructureSlotTypeTraits<StructureSlotType::eMap> {
		constexpr static string_view canonical = "Map"sv;

		constexpr static s64 const_size = 0LL;
	};

	template <>
	struct StructureSlotTypeTraits<StructureSlotType::eMapEntry> {
		constexpr static string_view canonical = "MapEntry"sv;

		constexpr static s64 const_size = 0LL;
	};

	template <>
	struct StructureSlotTypeTraits<StructureSlotType::eRecord> {
		constexpr static string_view canonical = "Record"sv;

		constexpr static s64 const_size = 0LL;
	};

	template <>
	struct StructureSlotTypeTraits<StructureSlotType::eStruct> {
		constexpr static string_view canonical = "Struct"sv;

		constexpr static s64 const_size = 0LL;
	};

	template <>
	struct StructureSlotTypeTraits<StructureSlotType::eSeq> {
		constexpr static string_view canonical = "Seq"sv;

		constexpr static s64 const_size = 0LL;
	};
}
