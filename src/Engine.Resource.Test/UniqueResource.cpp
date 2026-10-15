#include "pch.h"

#include <Engine.Resource/Manage/UniqueResource.hpp>

using namespace hg;
using namespace hg::engine::resource;

namespace ResourceModule {
	TEST(UniqueResourceTest, DefaultIsNotLoaded) {
		UniqueResource<int> res {};

		EXPECT_FALSE(res.isLoaded());
		EXPECT_EQ(res.loaded(), 0u);
	}

	TEST(UniqueResourceTest, InPlaceConstructedIsLoaded) {
		UniqueResource<int> res { std::in_place };

		EXPECT_TRUE(res.isLoaded());
	}

	TEST(UniqueResourceTest, WriteFlagGrantsMutableAccess) {
		UniqueResource<int> res { 5 };

		{
			auto guard = res.acquire(ResourceUsageFlag::eWrite);
			EXPECT_TRUE(guard.owns());
			ASSERT_NE(guard.mut(), nullptr);
			*static_cast<int*>(guard.mut().get()) = 9;
		}

		auto guard = res.acquire(ResourceUsageFlag::eRead);
		ASSERT_NE(guard.imm(), nullptr);
		EXPECT_EQ(*static_cast<const int*>(guard.imm().get()), 9);
	}

	TEST(UniqueResourceTest, ReadFlagDoesNotGrantMutableAccess) {
		UniqueResource<int> res { 5 };

		auto guard = res.acquire(ResourceUsageFlag::eRead);

		EXPECT_NE(guard.imm(), nullptr);
		EXPECT_EQ(guard.mut(), nullptr);
	}

	namespace {
		// ManageGuard has no public default constructor: an acquired-then-released guard serves as empty out-parameter.
		[[nodiscard]] ManageGuard emptyGuard(ref<UniqueResource<int>> res_) {
			auto guard = res_.acquire(ResourceUsageFlag::eDefault);
			guard.release();
			return guard;
		}
	}

	TEST(UniqueResourceTest, TryAcquireFailsWhileHeld) {
		UniqueResource<int> res { 1 };
		auto out = emptyGuard(res);

		auto first = res.acquire(ResourceUsageFlag::eDefault);

		EXPECT_FALSE(res.try_acquire(out, ResourceUsageFlag::eDefault));
		EXPECT_FALSE(out.owns());
	}

	TEST(UniqueResourceTest, TryAcquireSucceedsAfterRelease) {
		UniqueResource<int> res { 1 };
		auto out = emptyGuard(res);

		{
			auto first = res.acquire(ResourceUsageFlag::eDefault);
		}

		EXPECT_TRUE(res.try_acquire(out, ResourceUsageFlag::eDefault));
		EXPECT_TRUE(out.owns());
	}

	TEST(UniqueResourceTest, ExplicitReleaseFreesResource) {
		UniqueResource<int> res { 1 };
		auto out = emptyGuard(res);

		auto first = res.acquire(ResourceUsageFlag::eDefault);
		first.release();

		EXPECT_TRUE(res.try_acquire(out, ResourceUsageFlag::eDefault));
	}

	TEST(UniqueResourceTest, DefaultAssociationIsNull) {
		UniqueResource<int> res { 1 };
		EXPECT_EQ(res.getAssociation(), nullptr);
	}
}
