#include "pch.h"

/**/

import Engine.Common;

using namespace hg;

namespace Common {
	namespace {
		struct Keyed {
			int key;
			int tag;

			constexpr bool operator==(const Keyed&) const = default;
		};

		constexpr auto byKey = [](const Keyed& left_, const Keyed& right_) {
			return left_.key < right_.key;
		};
	}

	TEST(Algo, FindIfReturnsFirstMatchOrEnd) {
		constexpr std::array<int, 4> values { 1, 4, 3, 4 };

		static_assert(FindIf(values, [](int v_) { return v_ == 4; }) == values.begin() + 1);
		static_assert(FindIf(values, [](int v_) { return v_ > 9; }) == values.end());
		EXPECT_EQ(FindIf(values.begin(), values.begin(), [](int) { return true; }), values.begin());
	}

	TEST(Algo, CountOfCountsEqualElements) {
		constexpr std::array<char, 5> path { 'a', '/', 'b', '/', 'c' };

		static_assert(CountOf(path, '/') == 2uLL);
		EXPECT_EQ(CountOf(std::string {}, '/'), 0uLL);
	}

	TEST(Algo, LowerBoundFindsFirstNotLess) {
		constexpr std::array<int, 5> sorted { 1, 3, 3, 5, 7 };

		static_assert(LowerBound(sorted, 3) == sorted.begin() + 1);
		static_assert(LowerBound(sorted, 0) == sorted.begin());
		static_assert(LowerBound(sorted, 8) == sorted.end());
		static_assert(LowerBound(sorted.begin(), sorted.begin(), 3) == sorted.begin());

		constexpr std::array<Keyed, 3> keyed { Keyed { .key = 2, .tag = 0 }, Keyed { .key = 4, .tag = 1 }, Keyed { .key = 6, .tag = 2 } };
		static_assert(LowerBound(keyed, 4, LessFn {}, [](const Keyed& k_) { return k_.key; })->tag == 1);
	}

	TEST(Algo, CopyAndTransformWriteThrough) {
		constexpr auto doubled = [] {
			std::array<int, 3> src { 1, 2, 3 };
			std::array<int, 3> dst {};
			const auto end = TransformTo(src, dst.begin(), [](int v_) { return v_ * 2; });
			return end == dst.end() ? dst : std::array<int, 3> {};
		}();
		static_assert(doubled == std::array<int, 3> { 2, 4, 6 });

		constexpr auto copied = [] {
			std::array<int, 2> dst {};
			CopyTo(std::array<int, 2> { 7, 8 }, dst.begin());
			return dst;
		}();
		static_assert(copied == std::array<int, 2> { 7, 8 });
	}

	TEST(Algo, MergeIsStableAndKeepsLeftOnTies) {
		constexpr auto merged = [] {
			std::array<Keyed, 3> left { Keyed { .key = 1, .tag = 0 }, Keyed { .key = 2, .tag = 0 }, Keyed { .key = 2, .tag = 1 } };
			std::array<Keyed, 3> right { Keyed { .key = 0, .tag = 10 }, Keyed { .key = 2, .tag = 10 }, Keyed { .key = 3, .tag = 10 } };
			std::array<Keyed, 6> out {};
			Merge(left.begin(), left.end(), right.begin(), right.end(), out.begin(), byKey);
			return out;
		}();

		static_assert(
			merged == std::array<Keyed, 6> {
				Keyed { .key = 0, .tag = 10 }, Keyed { .key = 1, .tag = 0 }, Keyed { .key = 2, .tag = 0 }, Keyed { .key = 2, .tag = 1 }, Keyed { .key = 2, .tag = 10 }, Keyed { .key = 3, .tag = 10 }
			}
		);
	}

	TEST(Algo, SliceOfSplitsEvenlyAndCoversEveryIndexOnce) {
		// 10 into 4: sizes 3, 3, 2, 2
		static_assert(sliceOf(10u, 0u, 4u).begin == 0u && sliceOf(10u, 0u, 4u).end == 3u);
		static_assert(sliceOf(10u, 1u, 4u).begin == 3u && sliceOf(10u, 1u, 4u).end == 6u);
		static_assert(sliceOf(10u, 3u, 4u).begin == 8u && sliceOf(10u, 3u, 4u).end == 10u);
		static_assert(sliceOf(5, 0, 1).begin == 0 && sliceOf(5, 0, 1).end == 5);
		// more parts than elements: the tail parts are empty
		static_assert(sliceOf(2u, 1u, 4u).begin == 1u && sliceOf(2u, 1u, 4u).end == 2u);
		static_assert(sliceOf(2u, 3u, 4u).begin == 2u && sliceOf(2u, 3u, 4u).end == 2u);
		// the callback's u16 parts against a size_t total, and u16 throughout (promotes to int)
		static_assert(sliceOf(std::size_t { 10 }, u16 { 1 }, u16 { 4 }).end == std::size_t { 6 });
		static_assert(sliceOf(u16 { 10 }, u16 { 3 }, u16 { 4 }).begin == u16 { 8 });

		for (u32 total = 0; total < 40u; ++total) {
			for (u32 count = 1; count < 12u; ++count) {
				u32 next = 0;
				for (u32 index = 0; index < count; ++index) {
					const auto [begin, end] = sliceOf(total, index, count);
					EXPECT_EQ(begin, next) << total << " " << index << " " << count;
					EXPECT_LE(end - begin, total / count + 1u);
					next = end;
				}
				EXPECT_EQ(next, total);
			}
		}
	}

	TEST(Algo, SliceOfRangeIsSpanOrSubrange) {
		std::array<int, 7> values { 0, 1, 2, 3, 4, 5, 6 };
		const auto part = sliceOf(values, 1u, 3u);// 7 into 3: sizes 3, 2, 2
		static_assert(std::same_as<decltype(part), const std::span<int>>);
		ASSERT_EQ(part.size(), 2uz);
		EXPECT_EQ(part[0], 3);

		std::deque<int> queue { 0, 1, 2, 3, 4 };
		const auto tail = sliceOf(queue, 1u, 2u);
		EXPECT_EQ(std::ranges::distance(tail), 2);
		EXPECT_EQ(*tail.begin(), 3);
	}
}
