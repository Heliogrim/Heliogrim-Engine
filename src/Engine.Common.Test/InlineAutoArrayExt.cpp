#include "pch.h"

#include <vector>
#include <Engine.Common/Collection/InlineAutoArray.hpp>

using namespace hg;

namespace Common::Collection {
	namespace {
		inline int liveObjects = 0;

		struct Tracked {
			int value;

			explicit Tracked(const int value_) :
				value(value_) { ++liveObjects; }

			Tracked(Tracked&& other_) noexcept :
				value(other_.value) { ++liveObjects; }

			Tracked(const Tracked&) = delete;

			~Tracked() { --liveObjects; }
		};

		struct TrackedFixture :
			public ::testing::Test {
			void SetUp() override { liveObjects = 0; }
		};

		template <size_t Cap_>
		[[nodiscard]] std::vector<size_t> toVector(const InlineAutoArray<size_t, Cap_>& array_) {
			std::vector<size_t> out {};
			for (const auto& value : array_) { out.push_back(value); }
			return out;
		}
	}

	TEST(InlineAutoArrayExt, StartsEmpty) {
		InlineAutoArray<size_t, 2> array {};

		EXPECT_TRUE(array.empty());
		EXPECT_EQ(array.size(), 0uLL);
		EXPECT_EQ(array.size(), 0uLL);
	}

	TEST(InlineAutoArrayExt, ValueConstructorFillsInline) {
		const InlineAutoArray<size_t, 3> array { size_t { 1 }, size_t { 2 } };

		EXPECT_EQ(array.size(), 2uLL);
		EXPECT_EQ(array.front(), 1uLL);
		EXPECT_EQ(array.back(), 2uLL);
	}

	TEST(InlineAutoArrayExt, FrontAndBackWithinInline) {
		InlineAutoArray<size_t, 4> array {};
		array.emplace_back(10);
		array.emplace_back(20);

		EXPECT_EQ(array.front(), 10uLL);
		EXPECT_EQ(array.back(), 20uLL);

		array.back() = 21;
		array.front() = 11;
		EXPECT_EQ(array.front(), 11uLL);
		EXPECT_EQ(array.back(), 21uLL);
	}

	TEST(InlineAutoArrayExt, BackFollowsOverflowAndPop) {
		InlineAutoArray<size_t, 2> array {};
		array.emplace_back(1);
		array.emplace_back(2);
		array.emplace_back(3);
		array.emplace_back(4);

		EXPECT_EQ(array.back(), 4uLL);
		array.pop_back();
		EXPECT_EQ(array.back(), 3uLL);
		array.pop_back();
		EXPECT_EQ(array.back(), 2uLL);
		EXPECT_EQ(array.size(), 2uLL);
	}

	TEST(InlineAutoArrayExt, IterationPreservesInsertionOrderAcrossOverflow) {
		InlineAutoArray<size_t, 2> array {};
		for (size_t i = 0; i < 6; ++i) { array.emplace_back(i); }

		EXPECT_EQ(toVector(array), (std::vector<size_t> { 0, 1, 2, 3, 4, 5 }));
	}

	TEST(InlineAutoArrayExt, PopUntilEmpty) {
		InlineAutoArray<size_t, 2> array {};
		for (size_t i = 0; i < 5; ++i) { array.emplace_back(i); }

		for (size_t i = 0; i < 5; ++i) { array.pop_back(); }

		EXPECT_TRUE(array.empty());
		EXPECT_EQ(array.size(), 0uLL);
	}

	TEST(InlineAutoArrayExt, ReusableAfterDraining) {
		InlineAutoArray<size_t, 2> array {};
		for (size_t i = 0; i < 4; ++i) { array.emplace_back(i); }
		for (size_t i = 0; i < 4; ++i) { array.pop_back(); }

		array.emplace_back(7);
		array.emplace_back(8);
		array.emplace_back(9);

		EXPECT_EQ(toVector(array), (std::vector<size_t> { 7, 8, 9 }));
	}

	TEST(InlineAutoArrayExt, ClearEmptiesAndAllowsReuse) {
		InlineAutoArray<size_t, 2> array {};
		for (size_t i = 0; i < 4; ++i) { array.emplace_back(i); }

		array.clear();

		EXPECT_TRUE(array.empty());
		array.emplace_back(5);
		EXPECT_EQ(array.size(), 1uLL);
		EXPECT_EQ(array.front(), 5uLL);
	}


	TEST(InlineAutoArrayExt, MoveConstructTransfersInlineElements) {
		InlineAutoArray<size_t, 4> source {};
		source.emplace_back(1);
		source.emplace_back(2);

		InlineAutoArray<size_t, 4> target { std::move(source) };

		EXPECT_EQ(toVector(target), (std::vector<size_t> { 1, 2 }));
		EXPECT_TRUE(source.empty());
	}

	TEST(InlineAutoArrayExt, MoveConstructTransfersExternalElements) {
		InlineAutoArray<size_t, 2> source {};
		for (size_t i = 0; i < 5; ++i) { source.emplace_back(i); }

		InlineAutoArray<size_t, 2> target { std::move(source) };

		EXPECT_EQ(toVector(target), (std::vector<size_t> { 0, 1, 2, 3, 4 }));
		EXPECT_TRUE(source.empty());
	}

	using InlineAutoArrayLifetime = TrackedFixture;

	TEST_F(InlineAutoArrayLifetime, DestructorReleasesInlineAndExternal) {
		{
			InlineAutoArray<Tracked, 2> array {};
			for (int i = 0; i < 5; ++i) { array.emplace_back(i); }
			EXPECT_GE(liveObjects, 5);
		}
		EXPECT_EQ(liveObjects, 0);
	}

	TEST_F(InlineAutoArrayLifetime, PopBackDestroysElements) {
		InlineAutoArray<Tracked, 2> array {};
		for (int i = 0; i < 4; ++i) { array.emplace_back(i); }

		array.pop_back();
		array.pop_back();
		array.pop_back();

		EXPECT_EQ(array.size(), 1uLL);
		EXPECT_EQ(liveObjects, 1);
	}

	TEST_F(InlineAutoArrayLifetime, ClearDestroysElements) {
		InlineAutoArray<Tracked, 2> array {};
		for (int i = 0; i < 4; ++i) { array.emplace_back(i); }

		array.clear();

		EXPECT_EQ(liveObjects, 0);
	}

	TEST_F(InlineAutoArrayLifetime, MoveConstructDoesNotLeakOrDuplicate) {
		{
			InlineAutoArray<Tracked, 2> source {};
			for (int i = 0; i < 4; ++i) { source.emplace_back(i); }

			InlineAutoArray<Tracked, 2> target { std::move(source) };

			EXPECT_EQ(target.size(), 4uLL);
			EXPECT_EQ(liveObjects, 4);
		}
		EXPECT_EQ(liveObjects, 0);
	}
}
