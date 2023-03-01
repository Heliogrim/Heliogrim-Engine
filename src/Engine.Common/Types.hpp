#pragma once

namespace hg {
    typedef signed char s8;
    typedef short s16;
    typedef int s32;
    typedef long long s64;

    typedef long L;
    typedef long long LL;

    typedef unsigned char u8;
    typedef unsigned short u16;
    typedef unsigned int u32;
    typedef unsigned long long u64;

    typedef unsigned long uL;
    typedef unsigned long long uLL;

    /**/

    typedef s64 streamoff;
    typedef streamoff streampos;
    typedef s64 streamsize;

    /**/

    constexpr s8 s8_min = -127i8 - 1;
    constexpr s16 s16_min = -32767i16 - 1;
    constexpr s32 s32_min = -2147483647i32 - 1;
    constexpr s64 s64_min = -9223372036854775807i64 - 1;

    constexpr s8 s8_max = 127i8;
    constexpr s16 s16_max = 32767i16;
    constexpr s32 s32_max = 2147483647i32;
    constexpr s64 s64_max = 9223372036854775807i64;

    constexpr u8 u8_max = 0xFFui8;
    constexpr u16 u16_max = 0xFFFFui16;
    constexpr u32 u32_max = 0xFFFFFFFFui32;
    constexpr u64 u64_max = 0xFFFFFFFFFFFFFFFFui64;

    template <typename Numeric>
    struct limits { };

    template <>
    struct limits<s8> {
        constexpr static s8 max = s8_max;
        constexpr static s8 min = s8_min;
    };

    template <>
    struct limits<s16> {
        constexpr static s16 max = s16_max;
        constexpr static s16 min = s16_min;
    };

    template <>
    struct limits<s32> {
        constexpr static s32 max = s32_max;
        constexpr static s32 min = s32_min;
    };

    template <>
    struct limits<s64> {
        constexpr static s64 max = s64_max;
        constexpr static s64 min = s64_min;
    };

    template <>
    struct limits<u8> {
        constexpr static u8 max = u8_max;
        constexpr static u8 min = 0x0i8;
    };

    template <>
    struct limits<u16> {
        constexpr static u16 max = u16_max;
        constexpr static u16 min = 0x0i16;
    };

    template <>
    struct limits<u32> {
        constexpr static u32 max = u32_max;
        constexpr static u32 min = 0x0i32;
    };

    template <>
    struct limits<u64> {
        constexpr static u64 max = u64_max;
        constexpr static u64 min = 0x0i64;
    };
}

/*                                                  */
/* Special Type Definitions (Conflict with _STD)    */
/*                                                  */

namespace hg::_ {
    // @see <cstddef>
    enum class byte : u8 {};
}
