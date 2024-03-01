#pragma once

#include <Engine.Common/Types.hpp>

namespace hg::engine::serialization::layout {
    enum class LayoutDefineValueType {
        eNone,
        eInt8,
        eInt16,
        eInt32,
        eInt64,
        eUInt8,
        eUInt16,
        eUInt32,
        eUInt64,
        eFloat,
        eDouble,
        eBool,
        //
        eSpan,
        eSlice,
        eObject
    };

    template <LayoutDefineValueType ValueType_>
    struct LayoutDefineValueTrait {
        using type = void;

        inline static constexpr u64 size = 0uLL;
        inline static constexpr u64 padding = 0uLL;

        inline static constexpr bool is_inline = false;
        inline static constexpr bool is_constant_size = false;

        inline static constexpr bool has_subtype = false;
    };

    template <>
    struct LayoutDefineValueTrait<LayoutDefineValueType::eInt8> {
        using type = s8;

        inline static constexpr u64 size = sizeof(type);
        inline static constexpr u64 padding = 0uLL;

        inline static constexpr bool is_inline = true;
        inline static constexpr bool is_constant_size = true;

        inline static constexpr bool has_subtype = false;
    };

    template <>
    struct LayoutDefineValueTrait<LayoutDefineValueType::eInt16> {
        using type = s16;

        inline static constexpr u64 size = sizeof(type);
        inline static constexpr u64 padding = 0uLL;

        inline static constexpr bool is_inline = true;
        inline static constexpr bool is_constant_size = true;

        inline static constexpr bool has_subtype = false;
    };

    template <>
    struct LayoutDefineValueTrait<LayoutDefineValueType::eInt32> {
        using type = s32;

        inline static constexpr u64 size = sizeof(type);
        inline static constexpr u64 padding = 0uLL;

        inline static constexpr bool is_inline = true;
        inline static constexpr bool is_constant_size = true;

        inline static constexpr bool has_subtype = false;
    };

    template <>
    struct LayoutDefineValueTrait<LayoutDefineValueType::eInt64> {
        using type = s64;

        inline static constexpr u64 size = sizeof(type);
        inline static constexpr u64 padding = 0uLL;

        inline static constexpr bool is_inline = true;
        inline static constexpr bool is_constant_size = true;

        inline static constexpr bool has_subtype = false;
    };

    template <>
    struct LayoutDefineValueTrait<LayoutDefineValueType::eUInt8> {
        using type = u8;

        inline static constexpr u64 size = sizeof(type);
        inline static constexpr u64 padding = 0uLL;

        inline static constexpr bool is_inline = true;
        inline static constexpr bool is_constant_size = true;

        inline static constexpr bool has_subtype = false;
    };

    template <>
    struct LayoutDefineValueTrait<LayoutDefineValueType::eUInt16> {
        using type = u16;

        inline static constexpr u64 size = sizeof(type);
        inline static constexpr u64 padding = 0uLL;

        inline static constexpr bool is_inline = true;
        inline static constexpr bool is_constant_size = true;

        inline static constexpr bool has_subtype = false;
    };

    template <>
    struct LayoutDefineValueTrait<LayoutDefineValueType::eUInt32> {
        using type = u32;

        inline static constexpr u64 size = sizeof(type);
        inline static constexpr u64 padding = 0uLL;

        inline static constexpr bool is_inline = true;
        inline static constexpr bool is_constant_size = true;

        inline static constexpr bool has_subtype = false;
    };

    template <>
    struct LayoutDefineValueTrait<LayoutDefineValueType::eUInt64> {
        using type = u64;

        inline static constexpr u64 size = sizeof(type);
        inline static constexpr u64 padding = 0uLL;

        inline static constexpr bool is_inline = true;
        inline static constexpr bool is_constant_size = true;

        inline static constexpr bool has_subtype = false;
    };

    template <>
    struct LayoutDefineValueTrait<LayoutDefineValueType::eFloat> {
        using type = float;

        inline static constexpr u64 size = sizeof(type);
        inline static constexpr u64 padding = 0uLL;

        inline static constexpr bool is_inline = true;
        inline static constexpr bool is_constant_size = true;

        inline static constexpr bool has_subtype = false;
    };

    template <>
    struct LayoutDefineValueTrait<LayoutDefineValueType::eDouble> {
        using type = double;

        inline static constexpr u64 size = sizeof(type);
        inline static constexpr u64 padding = 0uLL;

        inline static constexpr bool is_inline = true;
        inline static constexpr bool is_constant_size = true;

        inline static constexpr bool has_subtype = false;
    };

    template <>
    struct LayoutDefineValueTrait<LayoutDefineValueType::eBool> {
        using type = bool;

        inline static constexpr u64 size = sizeof(bool);
        inline static constexpr u64 padding = 7uLL;

        inline static constexpr bool is_inline = true;
        inline static constexpr bool is_constant_size = true;

        inline static constexpr bool has_subtype = false;
    };

    template <>
    struct LayoutDefineValueTrait<LayoutDefineValueType::eSpan> {
        using type = void;

        inline static constexpr bool is_inline = true;
        inline static constexpr bool is_constant_size = false;

        inline static constexpr bool has_subtype = true;
    };

    template <>
    struct LayoutDefineValueTrait<LayoutDefineValueType::eSlice> {
        using type = void;

        inline static constexpr u64 size = sizeof(std::ptrdiff_t) * 2uLL;

        inline static constexpr bool is_inline = false;
        inline static constexpr bool is_constant_size = false;

        inline static constexpr bool has_subtype = true;
    };

    template <>
    struct LayoutDefineValueTrait<LayoutDefineValueType::eObject> {
        using type = void;

        inline static constexpr bool is_inline = true;
        inline static constexpr bool is_constant_size = true;

        inline static constexpr bool has_subtype = true;
    };
}
