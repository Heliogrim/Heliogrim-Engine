#pragma once

namespace hg {
	#if defined(_MSC_VER)

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

	#else

	typedef signed char s8;
	typedef short s16;
	typedef int s32;
	typedef long long s64;

	typedef long L;
	typedef long long LL;

	typedef unsigned char u8;
	typedef unsigned short u16;
	typedef unsigned int u32;
	typedef unsigned long int u64;

	#endif

	/**/

	typedef float f32;
	typedef double f64;

	/**/

	typedef u64 size_t;
	typedef u64 align_t;

	/**/

	typedef s64 streamoff;
	typedef streamoff streampos;
	typedef s64 streamsize;

	/**/

	#if defined(_MSC_VER)

	constexpr s8 s8_min = -127i8 - 1;
	constexpr s16 s16_min = -32767i16 - 1;
	constexpr s32 s32_min = -2147483647i32 - 1;
	constexpr s64 s64_min = -9223372036854775807i64 - 1;

	constexpr s8 s8_max = 127i8;
	constexpr s16 s16_max = 32767i16;
	constexpr s32 s32_max = 2147483647i32;
	constexpr s64 s64_max = 9223372036854775807i64;

	constexpr u8 u8_min = 0x0ui8;
	constexpr u16 u16_min = 0x0ui16;
	constexpr u32 u32_min = 0x0ui32;
	constexpr u64 u64_min = 0x0ui64;

	constexpr u8 u8_max = 0xFFui8;
	constexpr u16 u16_max = 0xFFFFui16;
	constexpr u32 u32_max = 0xFFFFFFFFui32;
	constexpr u64 u64_max = 0xFFFFFFFFFFFFFFFFui64;

	#else

	constexpr s8 s8_min = -127;
	constexpr s16 s16_min = -32767;
	constexpr s32 s32_min = -2147483647L;
	constexpr s64 s64_min = -9223372036854775807LL;

	constexpr s8 s8_max = 127;
	constexpr s16 s16_max = 32767;
	constexpr s32 s32_max = 2147483647L;
	constexpr s64 s64_max = 9223372036854775807LL;

	constexpr u8 u8_min = 0x0u;
	constexpr u16 u16_min = 0x0uL;
	constexpr u32 u32_min = 0x0uL;
	constexpr u64 u64_min = 0x0uLL;

	constexpr u8 u8_max = 0xFFu;
	constexpr u16 u16_max = 0xFFFFuL;
	constexpr u32 u32_max = 0xFFFFFFFFuL;
	constexpr u64 u64_max = 0xFFFFFFFFFFFFFFFFuLL;

	#endif

	template <typename Numeric>
	struct limits {};

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
		constexpr static u8 min = u8_min;
	};

	template <>
	struct limits<u16> {
		constexpr static u16 max = u16_max;
		constexpr static u16 min = u16_min;
	};

	template <>
	struct limits<u32> {
		constexpr static u32 max = u32_max;
		constexpr static u32 min = u32_min;
	};

	template <>
	struct limits<u64> {
		constexpr static u64 max = u64_max;
		constexpr static u64 min = u64_min;
	};
}

/*                                                  */
/* Special Type Definitions (Conflict with _STD)    */
/*                                                  */

namespace hg::_ {
	// @see <cstddef>
	enum class byte : u8 {};
}
