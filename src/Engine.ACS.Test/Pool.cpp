#include "pch.h"

#include <algorithm>
#include <string>
#include <Engine.ACS/Pool.hpp>

using namespace hg;
using namespace hg::engine::acs;

namespace Acs {
	namespace {
		constexpr u64 invalidKey = ~0uLL;

		// Pool is a per-type singleton: every test uses its own tag to stay isolated.
		template <int Tag_>
		struct Item {
			int value = 0;

			Item() = default;

			explicit Item(const int value_) :
				value(value_) {}
		};

		template <int Tag_>
		using TestPool = Pool<u64, Item<Tag_>, invalidKey>;
	}

	TEST(AcsPool, StartsEmpty) {
		auto& pool = TestPool<1>::getOrCreate();

		EXPECT_EQ(pool.get(1uLL), nullptr);
	}

	TEST(AcsPool, GetOrCreateReturnsSingleton) {
		EXPECT_EQ(&TestPool<2>::getOrCreate(), &TestPool<2>::getOrCreate());
		EXPECT_EQ(&TestPool<2>::getOrCreate(), &TestPool<2>::get());
	}

	TEST(AcsPool, InsertDefaultConstructs) {
		auto& pool = TestPool<3>::getOrCreate();
		pool.insert(7uLL);

		ASSERT_NE(pool.get(7uLL), nullptr);
		EXPECT_EQ(pool.get(7uLL)->value, 0);
	}

	TEST(AcsPool, InsertForwardsConstructorArguments) {
		auto& pool = TestPool<4>::getOrCreate();
		pool.insert(7uLL, 42);

		ASSERT_NE(pool.get(7uLL), nullptr);
		EXPECT_EQ(pool.get(7uLL)->value, 42);
	}

	TEST(AcsPool, EmplaceReportsInsertion) {
		auto& pool = TestPool<5>::getOrCreate();

		const auto first = pool.emplace(1uLL, 10);
		ASSERT_NE(first.first, nullptr);
		EXPECT_TRUE(first.second);

		const auto second = pool.emplace(1uLL, 20);
		EXPECT_FALSE(second.second);
		EXPECT_EQ(pool.get(1uLL)->value, 10);
	}

	TEST(AcsPool, EraseRemovesEntry) {
		auto& pool = TestPool<7>::getOrCreate();
		pool.insert(1uLL, 1);
		pool.insert(2uLL, 2);

		pool.erase(1uLL);

		EXPECT_EQ(pool.get(1uLL), nullptr);
		ASSERT_NE(pool.get(2uLL), nullptr);
		EXPECT_EQ(pool.get(2uLL)->value, 2);
	}

	TEST(AcsPool, ManyEntriesStayAddressable) {
		auto& pool = TestPool<9>::getOrCreate();

		for (u64 i = 0; i < 1000uLL; ++i) { pool.insert(i, static_cast<int>(i) * 2); }

		for (u64 i = 0; i < 1000uLL; ++i) {
			const auto* const item = pool.get(i);
			ASSERT_NE(item, nullptr) << i;
			EXPECT_EQ(item->value, static_cast<int>(i) * 2);
		}

		for (u64 i = 0; i < 1000uLL; i += 2) { pool.erase(i); }
		EXPECT_EQ(pool.get(10uLL), nullptr);
		EXPECT_NE(pool.get(11uLL), nullptr);
	}
}
