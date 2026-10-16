#include "pch.h"

#include <string_view>
#include <Engine.Common/Hash/Fnv-1a.hpp>
#include <Engine.Common/Hash/Murmur3.hpp>

using namespace hg;
using hg::hash::operator""_fnv1a;

namespace Common {
	namespace {
		[[nodiscard]] u32 murmur32(const std::string_view str_, const u32 seed_ = 0uL) {
			u32 out = 0uL;
			hash::murmur3_x86_32(str_.data(), static_cast<u32>(str_.size()), seed_, &out);
			return out;
		}
	}

	// Reference vectors from the canonical MurmurHash3 implementation.
	TEST(Murmur3, x86_32KnownVectors) {
		EXPECT_EQ(murmur32(""), 0x00000000uL);
		EXPECT_EQ(murmur32("", 1uL), 0x514E28B7uL);
	}

	TEST(Murmur3, x86_32CanonicalVectors) {
		EXPECT_EQ(murmur32("test"), 0xBA6BD213uL);
		EXPECT_EQ(murmur32("Hello, world!", 0x9747B28CuL), 0x24884CBAuL);
	}

	TEST(Murmur3, x86_32SeedChangesResult) {
		EXPECT_NE(murmur32("abc", 0uL), murmur32("abc", 1uL));
	}

	TEST(Murmur3, x86_32TailLengthsAreDistinct) {
		// Exercise the 1..3 byte tail paths (<4 bytes).
		EXPECT_NE(murmur32("a"), murmur32("ab"));
		EXPECT_NE(murmur32("ab"), murmur32("abc"));
	}

	TEST(Murmur3, x64_128EmptyZeroSeedIsZero) {
		u64 out[2] { 1uLL, 1uLL };
		hash::murmur3_x64_128("", 0uL, 0uLL, out);
		EXPECT_EQ(out[0], 0uLL);
		EXPECT_EQ(out[1], 0uLL);
	}

	TEST(Murmur3, x64_128IsDeterministicAndInputSensitive) {
		u64 a[2] {}, b[2] {}, c[2] {};
		hash::murmur3_x64_128("heliogrim", 9uL, 7uLL, a);
		hash::murmur3_x64_128("heliogrim", 9uL, 7uLL, b);
		hash::murmur3_x64_128("heliogrin", 9uL, 7uLL, c);

		EXPECT_EQ(a[0], b[0]);
		EXPECT_EQ(a[1], b[1]);
		EXPECT_TRUE(a[0] != c[0] || a[1] != c[1]);
	}

	TEST(Murmur3, x64_64IsDeterministic) {
		u64 a = 0uLL, b = 0uLL;
		hash::murmur3_x64_64("heliogrim", 9uL, 3uL, a);
		hash::murmur3_x64_64("heliogrim", 9uL, 3uL, b);
		EXPECT_EQ(a, b);
		EXPECT_NE(a, 0uLL);
	}

	TEST(Fnv1a, EmptyIsOffsetBasis) {
		EXPECT_EQ(hash::fnv1a(std::string_view {}), 2166136261uLL);
	}

	TEST(Fnv1a, SingleCharMatchesFormula) {
		EXPECT_EQ(hash::fnv1a(std::string_view { "a" }), (2166136261uLL ^ 97uLL) * 16777619uLL);
	}

	TEST(Fnv1a, OverloadsAgree) {
		const std::string_view view { "heliogrim" };
		const string str { "heliogrim" };

		EXPECT_EQ(hash::fnv1a(view), hash::fnv1a(str));
		EXPECT_EQ(hash::fnv1a(view), hash::fnv1a(std::span<const char> { view.data(), view.size() }));
		EXPECT_EQ(hash::fnv1a(view), "heliogrim"_fnv1a);
	}

	TEST(Fnv1a, DistinguishesInputs) {
		EXPECT_NE(hash::fnv1a(std::string_view { "ab" }), hash::fnv1a(std::string_view { "ba" }));
	}

	TEST(Fnv1a, ConstexprEvaluable) {
		static_assert(hash::fnv1a(std::string_view {}) == 2166136261uLL);
	}
}
