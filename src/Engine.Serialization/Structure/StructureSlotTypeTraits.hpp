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
        inline constexpr static string_view canonical = "Undefined"sv;

        inline constexpr static s64 const_size = -1LL;
    };

    template <>
    struct StructureSlotTypeTraits<StructureSlotType::eU8> {
        inline constexpr static string_view canonical = "U8"sv;

        inline constexpr static s64 const_size = 1LL;
    };

    template <>
    struct StructureSlotTypeTraits<StructureSlotType::eU16> {
        inline constexpr static string_view canonical = "U16"sv;

        inline constexpr static s64 const_size = 2LL;
    };

    template <>
    struct StructureSlotTypeTraits<StructureSlotType::eU32> {
        inline constexpr static string_view canonical = "U32"sv;

        inline constexpr static s64 const_size = 4LL;
    };

    template <>
    struct StructureSlotTypeTraits<StructureSlotType::eU64> {
        inline constexpr static string_view canonical = "U64"sv;

        inline constexpr static s64 const_size = 8LL;
    };

    template <>
    struct StructureSlotTypeTraits<StructureSlotType::eBool> {
        inline constexpr static string_view canonical = "Bool"sv;

        inline constexpr static s64 const_size = StructureSlotTypeTraits<StructureSlotType::eU8>::const_size;
    };

    template <>
    struct StructureSlotTypeTraits<StructureSlotType::eS8> {
        inline constexpr static string_view canonical = "S8"sv;

        inline constexpr static s64 const_size = StructureSlotTypeTraits<StructureSlotType::eU8>::const_size;
    };

    template <>
    struct StructureSlotTypeTraits<StructureSlotType::eS16> {
        inline constexpr static string_view canonical = "S16"sv;

        inline constexpr static s64 const_size = StructureSlotTypeTraits<StructureSlotType::eU16>::const_size;
    };

    template <>
    struct StructureSlotTypeTraits<StructureSlotType::eS32> {
        inline constexpr static string_view canonical = "S32"sv;

        inline constexpr static s64 const_size = StructureSlotTypeTraits<StructureSlotType::eU32>::const_size;
    };

    template <>
    struct StructureSlotTypeTraits<StructureSlotType::eS64> {
        inline constexpr static string_view canonical = "S64"sv;

        inline constexpr static s64 const_size = StructureSlotTypeTraits<StructureSlotType::eU64>::const_size;
    };

    template <>
    struct StructureSlotTypeTraits<StructureSlotType::eFloat> {
        inline constexpr static string_view canonical = "Float"sv;

        inline constexpr static s64 const_size = 4LL;
    };

    template <>
    struct StructureSlotTypeTraits<StructureSlotType::eDouble> {
        inline constexpr static string_view canonical = "Double"sv;

        inline constexpr static s64 const_size = 8LL;
    };

    template <>
    struct StructureSlotTypeTraits<StructureSlotType::eString> {
        inline constexpr static string_view canonical = "String"sv;

        inline constexpr static s64 const_size = 0LL;
    };

    template <>
    struct StructureSlotTypeTraits<StructureSlotType::eSlice> {
        inline constexpr static string_view canonical = "Slice"sv;

        inline constexpr static s64 const_size = 0LL;
    };

    template <>
    struct StructureSlotTypeTraits<StructureSlotType::eMap> {
        inline constexpr static string_view canonical = "Map"sv;

        inline constexpr static s64 const_size = 0LL;
    };

    template <>
    struct StructureSlotTypeTraits<StructureSlotType::eMapEntry> {
        inline constexpr static string_view canonical = "MapEntry"sv;

        inline constexpr static s64 const_size = 0LL;
    };

    template <>
    struct StructureSlotTypeTraits<StructureSlotType::eRecord> {
        inline constexpr static string_view canonical = "Record"sv;

        inline constexpr static s64 const_size = 0LL;
    };

    template <>
    struct StructureSlotTypeTraits<StructureSlotType::eStruct> {
        inline constexpr static string_view canonical = "Struct"sv;

        inline constexpr static s64 const_size = 0LL;
    };

    template <>
    struct StructureSlotTypeTraits<StructureSlotType::eSeq> {
        inline constexpr static string_view canonical = "Seq"sv;

        inline constexpr static s64 const_size = 0LL;
    };
}
