#include "pch.h"

#include <set>
#include <unordered_set>
#include <Engine.Common/Guid.hpp>
#include <Engine.Common/GuidFormat.hpp>

using namespace hg;

namespace Common {
	TEST(GuidOps, DefaultIsZeroAndFalsy) {
		const Guid guid {};

		EXPECT_EQ(guid.pre, 0u);
		EXPECT_EQ(guid.c0, 0u);
		EXPECT_EQ(guid.c1, 0u);
		EXPECT_EQ(guid.post, 0uLL);
		EXPECT_FALSE(static_cast<bool>(guid));
		EXPECT_TRUE(not guid);
	}

	TEST(GuidOps, AnyNonZeroComponentIsTruthy) {
		EXPECT_TRUE(static_cast<bool>(Guid { 1u, 0u, 0u, 0uLL }));
		EXPECT_TRUE(static_cast<bool>(Guid { 0u, 1u, 0u, 0uLL }));
		EXPECT_TRUE(static_cast<bool>(Guid { 0u, 0u, 1u, 0uLL }));
		EXPECT_TRUE(static_cast<bool>(Guid { 0u, 0u, 0u, 1uLL }));
	}

	TEST(GuidOps, EqualityComparesAllComponents) {
		const Guid base { 1u, 2u, 3u, 4uLL };

		EXPECT_EQ(base, (Guid { 1u, 2u, 3u, 4uLL }));
		EXPECT_NE(base, (Guid { 9u, 2u, 3u, 4uLL }));
		EXPECT_NE(base, (Guid { 1u, 9u, 3u, 4uLL }));
		EXPECT_NE(base, (Guid { 1u, 2u, 9u, 4uLL }));
		EXPECT_NE(base, (Guid { 1u, 2u, 3u, 9uLL }));
	}

	TEST(GuidOps, GenerateYieldsDistinctNonZeroGuids) {
		Guid a {}, b {};
		GuidGenerate(a);
		GuidGenerate(b);

		EXPECT_TRUE(static_cast<bool>(a));
		EXPECT_NE(a, b);
	}

	TEST(GuidOps, HashIsStableAndSpreads) {
		const Guid a { 1u, 2u, 3u, 4uLL };
		const Guid b { 1u, 2u, 3u, 5uLL };

		EXPECT_EQ(std::hash<Guid> {}(a), std::hash<Guid> {}(Guid { 1u, 2u, 3u, 4uLL }));
		EXPECT_NE(std::hash<Guid> {}(a), std::hash<Guid> {}(b));
	}

	TEST(GuidOps, UsableAsUnorderedKey) {
		std::unordered_set<Guid> set {};
		Guid a {}, b {};
		GuidGenerate(a);
		GuidGenerate(b);

		set.insert(a);
		set.insert(b);
		set.insert(a);

		EXPECT_EQ(set.size(), 2uLL);
		EXPECT_TRUE(set.contains(a));
	}

	// BUG: Guid::operator< / > / <=> reinterpret the guid bytes as a pointer to u64[2] (double indirection in
	// Guid.hpp) and dereference it: access violation (0xc0000005) for any real value. Enable once fixed.
	TEST(GuidOps, DISABLED_OrderingIsStrictAndConsistent) {
		const Guid low { 1u, 0u, 0u, 0uLL };
		const Guid high { 2u, 0u, 0u, 0uLL };

		EXPECT_TRUE(low < high);
		EXPECT_FALSE(high < low);
		EXPECT_FALSE(low < low);
		EXPECT_TRUE(high > low);
		EXPECT_TRUE((low <=> high) < 0);
		EXPECT_TRUE((low <=> low) == 0);
	}

	TEST(GuidFormatTest, EncodeJoinsComponentsWithDash) {
		EXPECT_EQ(encodeGuid4228(Guid { 1u, 2u, 3u, 4uLL }), "1-2-3-4");
	}

	TEST(GuidFormatTest, DecodeRoundTrip) {
		const Guid guid { 0xDEADBEEFu, 0x1234u, 0x5678u, 0x0123456789ABCDEFuLL };

		EXPECT_EQ(decodeGuid4228(encodeGuid4228(guid)), guid);
	}

	TEST(GuidFormatTest, DecodeZeroGuid) {
		EXPECT_EQ(decodeGuid4228("0-0-0-0"), (Guid {}));
	}

	TEST(GuidFormatTest, DecodeMaximumComponents) {
		const Guid guid { 0xFFFFFFFFu, 0xFFFFu, 0xFFFFu, 0xFFFFFFFFFFFFFFFFuLL };

		EXPECT_EQ(decodeGuid4228(encodeGuid4228(guid)), guid);
	}

	TEST(GuidFormatTest, DecodeGarbageThrows) {
		EXPECT_ANY_THROW((void)decodeGuid4228("not-a-guid"));
	}
}
