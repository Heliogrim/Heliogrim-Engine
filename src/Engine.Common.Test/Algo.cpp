#include "pch.h"

/**/

#include <array>
#include <string>
#include <Engine.Common/Algo/Copy.hpp>
#include <Engine.Common/Algo/Search.hpp>

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
}
