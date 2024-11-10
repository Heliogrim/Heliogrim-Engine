#include "pch.h"
/**/

#include <Engine.Common/Collection/InlineAutoArray.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>

using namespace hg;

namespace Common::Collection {
	TEST(InlineAutoArray, ZeroSize) {
		//InlineAutoArray<size_t, 0> tmp {};
		// Result: Will not compile due to min capacity constraint
	}

	TEST(InlineAutoArray, SingleSize) {
		InlineAutoArray<size_t, 1> tmp {};

		EXPECT_EQ(tmp.size(), 0);
		tmp.emplace_back(0);
		EXPECT_EQ(tmp.size(), 1);
	}

	TEST(InlineAutoArray, InlineSize) {
		InlineAutoArray<size_t, 4> tmp {};

		EXPECT_EQ(tmp.size(), 0);
		tmp.emplace_back(0);
		tmp.emplace_back(1);
		tmp.emplace_back(2);
		EXPECT_EQ(tmp.size(), 3);
	}

	TEST(InlineAutoArray, OverflowSize) {
		InlineAutoArray<size_t, 4> tmp {};

		EXPECT_EQ(tmp.size(), 0);

		tmp.emplace_back(0);
		tmp.emplace_back(1);
		tmp.emplace_back(2);
		tmp.emplace_back(3);
		tmp.emplace_back(4);
		tmp.emplace_back(5);

		EXPECT_EQ(tmp.size(), 6);
	}

	TEST(InlineAutoArray, PopInlineBack) {

		InlineAutoArray<size_t, 4> tmp {};

		tmp.emplace_back(0);
		tmp.emplace_back(1);

		EXPECT_NE(*tmp.begin(), *(--tmp.end()));

		tmp.pop_back();

		EXPECT_EQ(tmp.size(), 1);
		EXPECT_EQ(*tmp.begin(), *(--tmp.end()));
	}

	TEST(InlineAutoArray, PopExternalBack) {

		InlineAutoArray<size_t, 1> tmp {};

		tmp.emplace_back(0);
		tmp.emplace_back(1);
		tmp.emplace_back(2);

		EXPECT_NE(*tmp.begin(), *(--tmp.end()));

		tmp.pop_back();

		EXPECT_EQ(tmp.size(), 2);
		EXPECT_NE(*tmp.begin(), *(--tmp.end()));
		EXPECT_EQ(*(++tmp.begin()), *(--tmp.end()));
	}

	TEST(InlineAutoArray, PopMixedBack) {

		InlineAutoArray<size_t, 2> tmp {};

		tmp.emplace_back(0);
		tmp.emplace_back(1);
		tmp.emplace_back(20);
		tmp.emplace_back(21);

		EXPECT_NE(*tmp.begin(), *(--tmp.end()));

		tmp.pop_back();
		tmp.pop_back();

		tmp.pop_back();
		tmp.pop_back();

		EXPECT_EQ(tmp.size(), 0);
	}

	TEST(InlineAutoArray, Iterate) {

		InlineAutoArray<size_t, 8> tmp {};

		for (size_t i = 0; i < 0xF; ++i) {
			tmp.emplace_back(i);
		}

		EXPECT_EQ(tmp.size(), 0xF);

		size_t c { 0 };
		for (const auto& entry : tmp) {
			EXPECT_EQ(entry, c++);
		}

		EXPECT_EQ(c, 0xF);
	}

	TEST(InlineAutoArray, Nested) {

		using inner_type = InlineAutoArray<smr<size_t>, 1>;
		using outer_type = InlineAutoArray<inner_type, 1>;

		outer_type tmp {};
		tmp.emplace_back(inner_type {});
	}
}
