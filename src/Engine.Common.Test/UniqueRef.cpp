#include "pch.h"

#include <Engine.Common/Managed/UniqueRef.hpp>

using namespace hg;

namespace Common {
	namespace {
		struct Counter {
			int released = 0;
		};
	}

	TEST(UniqueRef, defaultIsNull) {
		UniqueRef<int> ref {};
		EXPECT_TRUE(ref.is_null());
	}

	TEST(UniqueRef, releasesOnceOnDestruction) {
		int value = 7;
		Counter counter {};
		{
			UniqueRef ref { [&](auto&) { ++counter.released; }, value };
			EXPECT_FALSE(ref.is_null());
			EXPECT_EQ(*ref, 7);
		}
		EXPECT_EQ(counter.released, 1);
	}

	TEST(UniqueRef, moveConstructTransfersOwnership) {
		int value = 7;
		Counter counter {};
		{
			UniqueRef a { [&](auto&) { ++counter.released; }, value };
			UniqueRef b { ::hg::move(a) };

			EXPECT_TRUE(a.is_null());
			EXPECT_FALSE(b.is_null());
		}
		EXPECT_EQ(counter.released, 1);
	}

	TEST(UniqueRef, moveAssignReleasesPreviousTarget) {
		int first = 1;
		int second = 2;
		Counter counter {};
		{
			UniqueRef a { [&](auto&) { ++counter.released; }, first };
			UniqueRef b { [&](auto&) { ++counter.released; }, second };

			a = ::hg::move(b);
			EXPECT_EQ(counter.released, 1);
			EXPECT_EQ(*a, 2);
			EXPECT_TRUE(b.is_null());
		}
		EXPECT_EQ(counter.released, 2);
	}

	TEST(UniqueRef, selfMoveAssignKeepsValue) {
		int value = 7;
		Counter counter {};
		{
			UniqueRef a { [&](auto&) { ++counter.released; }, value };
			auto& alias = a;
			a = ::hg::move(alias);

			EXPECT_FALSE(a.is_null());
			EXPECT_EQ(counter.released, 0);
		}
		EXPECT_EQ(counter.released, 1);
	}

	TEST(UniqueRef, explicitReleaseDoesNotDoubleRelease) {
		int value = 7;
		Counter counter {};
		{
			UniqueRef ref { [&](auto&) { ++counter.released; }, value };
			ref.release();
			EXPECT_EQ(counter.released, 1);
		}
		EXPECT_EQ(counter.released, 1);
	}

	TEST(UniqueRef, arrowAccessesMembers) {
		Counter counter {};
		UniqueRef ref { [](auto&) {}, counter };

		ref->released = 3;
		EXPECT_EQ(counter.released, 3);
	}
}
