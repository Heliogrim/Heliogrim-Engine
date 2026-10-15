#include "pch.h"

#include <Engine.ACS/Traits.hpp>
#include <Engine.Common/Move.hpp>
#include <Heliogrim/Component/HierarchyComponent.hpp>

using namespace hg;

namespace HeliogrimApi {
	namespace {
		[[nodiscard]] HierarchyComponent make(ptr<HierarchyComponent> parent_ = nullptr, ptr<Actor> owner_ = nullptr) {
			return HierarchyComponent {
				generate_component_guid(),
				CachedActorPointer { .guid = invalid_actor_guid, .cached = owner_ },
				::hg::move(parent_)
			};
		}
	}

	TEST(HierarchyComponentTest, RootIsItself) {
		auto root = make();

		EXPECT_EQ(root.getParentComponent(), nullptr);
		EXPECT_EQ(root.getRootComponent(), &root);
	}

	TEST(HierarchyComponentTest, RootResolvesThroughParentChain) {
		auto root = make();
		auto mid = make(&root);
		auto leaf = make(&mid);

		EXPECT_EQ(leaf.getParentComponent(), &mid);
		EXPECT_EQ(leaf.getRootComponent(), &root);
		EXPECT_EQ(mid.getRootComponent(), &root);
	}

	TEST(HierarchyComponentTest, RootActorIsOwnerOfRoot) {
		auto* const owner = reinterpret_cast<ptr<Actor>>(static_cast<uintptr_t>(0x1000));
		auto root = make(nullptr, owner);
		auto child = make(&root, nullptr);

		EXPECT_EQ(root.getOwner(), owner);
		EXPECT_EQ(child.getRootActor(), owner);
	}

	TEST(HierarchyComponentTest, RootUniverseLocationIsLocalLocation) {
		auto root = make();
		root.getLocalTransform().setLocation(math::Location { 1.F, 2.F, 3.F });

		EXPECT_EQ(root.getUniverseLocation(), (math::Location { 1.F, 2.F, 3.F }));
	}

	TEST(HierarchyComponentTest, ChildUniverseLocationAddsParentTranslation) {
		auto root = make();
		root.getLocalTransform().setLocation(math::Location { 10.F, 0.F, 0.F });
		auto child = make(&root);
		child.getLocalTransform().setLocation(math::Location { 0.F, 5.F, 0.F });

		const auto loc = child.getUniverseLocation().into();

		EXPECT_NEAR(loc.x, 10.F, 1e-4F);
		EXPECT_NEAR(loc.y, 5.F, 1e-4F);
		EXPECT_NEAR(loc.z, 0.F, 1e-4F);
	}

	TEST(HierarchyComponentTest, RootRelativeSetterStoresLocation) {
		auto root = make();

		root.setRelativeLocationAndRotator(math::Location { 4.F, 5.F, 6.F }, math::Rotator {});

		EXPECT_EQ(root.getLocalTransform().location(), (math::Location { 4.F, 5.F, 6.F }));
	}

	// SUSPECTED BUG (unverified intent): for a child the requested location is discarded (`setLocationAndRotator({}, ...)`), only the rotator is applied.
	TEST(HierarchyComponentTest, DISABLED_ChildRelativeSetterStoresLocation) {
		auto root = make();
		auto child = make(&root);

		child.setRelativeLocationAndRotator(math::Location { 4.F, 5.F, 6.F }, math::Rotator {});

		EXPECT_EQ(child.getLocalTransform().location(), (math::Location { 4.F, 5.F, 6.F }));
	}
}
