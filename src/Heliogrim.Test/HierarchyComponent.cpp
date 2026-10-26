#include "pch.h"

#include <vector>
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

	/* The setter takes a pose in universe space; the stored local transform is relative to the parent. */

	TEST(HierarchyComponentTest, ChildSetterSubtractsUnrotatedParentTranslation) {
		auto root = make();
		root.getLocalTransform().setLocation(math::Location { 10.F, 0.F, 0.F });
		auto child = make(&root);

		// Universe (12, 3, 4) seen from a parent at (10, 0, 0) without rotation is (2, 3, 4).
		child.setRelativeLocationAndRotator(math::Location { 12.F, 3.F, 4.F }, math::Rotator {});

		const auto local = child.getLocalTransform().location().into();
		EXPECT_NEAR(local.x, 2.F, 1e-4F);
		EXPECT_NEAR(local.y, 3.F, 1e-4F);
		EXPECT_NEAR(local.z, 4.F, 1e-4F);
	}

	TEST(HierarchyComponentTest, ChildSetterRoundTripsThroughRotatedParent) {
		auto root = make();
		root.getLocalTransform().setLocationAndRotator(math::Location { 10.F, 0.F, 0.F }, math::Rotator { 0.F, 0.5F, 0.F });
		auto child = make(&root);

		// Whatever the parent's rotation does to the local values, the universe pose must be the requested one.
		child.setRelativeLocationAndRotator(math::Location { 10.F, 0.F, 5.F }, math::Rotator {});

		const auto universe = child.getUniverseLocation().into();
		EXPECT_NEAR(universe.x, 10.F, 1e-3F);
		EXPECT_NEAR(universe.y, 0.F, 1e-3F);
		EXPECT_NEAR(universe.z, 5.F, 1e-3F);
	}

	TEST(HierarchyComponentTest, ChildSetterDerivesLocalRotatorFromParentUniverseRotator) {
		auto root = make();
		root.getLocalTransform().setRotator(math::Rotator { 0.F, 0.5F, 0.F });
		auto child = make(&root);

		// Parent yaw 0.5 rad, requested universe yaw 0.8 rad => child yaw relative to parent is 0.3 rad.
		child.setRelativeLocationAndRotator(math::Location {}, math::Rotator { 0.F, 0.8F, 0.F });

		EXPECT_NEAR(child.getLocalTransform().rotator().yaw(), 0.3F, 1e-3F);
		EXPECT_NEAR(child.getUniverseRotator().yaw(), 0.8F, 1e-3F);
	}

	TEST(HierarchyComponentTest, UniverseRotatorAccumulatesAlongParentChain) {
		auto grand = make();
		grand.getLocalTransform().setRotator(math::Rotator { 0.F, 0.2F, 0.F });
		auto parent = make(&grand);
		parent.getLocalTransform().setRotator(math::Rotator { 0.F, 0.3F, 0.F });
		auto child = make(&parent);
		child.getLocalTransform().setRotator(math::Rotator { 0.F, 0.1F, 0.F });

		// Yaw adds up along the chain: 0.2 + 0.3 + 0.1 = 0.6 rad.
		EXPECT_NEAR(child.getUniverseRotator().yaw(), 0.6F, 1e-3F);
	}

	/* Universe transform: the whole chain is folded in one walk; nested manual rotations act as the reference. */

	TEST(HierarchyComponentTest, UniverseLocationUnderRotatedAndScaledParent) {
		auto root = make();
		const auto rootRotator = math::Rotator { 0.3F, 0.7F, -0.2F };
		root.getLocalTransform().setLocationAndRotator(math::Location { 10.F, 0.F, 0.F }, math::Rotator { rootRotator });
		root.getLocalTransform().setScale(math::Vector3f32 { 2.F, 1.F, 0.5F });
		auto child = make(&root);
		child.getLocalTransform().setLocation(math::Location { 1.F, 2.F, 3.F });

		// The child offset (1, 2, 3) is scaled by the parent first: (2, 2, 1.5); then rotated, then moved to (10, 0, 0).
		const auto scaled = math::fvec3 { 2.F, 2.F, 1.5F };
		const auto expected = rootRotator.rotated(scaled) + math::fvec3 { 10.F, 0.F, 0.F };

		const auto loc = child.getUniverseLocation().into();
		EXPECT_NEAR(loc.x, expected.x, 1e-3F);
		EXPECT_NEAR(loc.y, expected.y, 1e-3F);
		EXPECT_NEAR(loc.z, expected.z, 1e-3F);
	}

	TEST(HierarchyComponentTest, UniverseTransformMatchesNestedRotationsOverThreeLevels) {
		const auto r1 = math::Rotator { 0.3F, 0.4F, 0.2F };
		const auto r2 = math::Rotator { -0.2F, 0.5F, 0.1F };
		const auto r3 = math::Rotator { 0.1F, -0.3F, 0.6F };

		auto grand = make();
		grand.getLocalTransform().setLocationAndRotator(math::Location { 1.F, 0.F, 0.F }, math::Rotator { r1 });
		auto parent = make(&grand);
		parent.getLocalTransform().setLocationAndRotator(math::Location { 0.F, 2.F, 0.F }, math::Rotator { r2 });
		auto child = make(&parent);
		child.getLocalTransform().setLocationAndRotator(math::Location { 0.F, 0.F, 3.F }, math::Rotator { r3 });

		// Location: every level rotates everything below it, i.e. t1 + R1 * (t2 + R2 * t3).
		const auto expectedLoc = math::fvec3 { 1.F, 0.F, 0.F } +
			r1.rotated(math::fvec3 { 0.F, 2.F, 0.F } + r2.rotated(math::fvec3 { 0.F, 0.F, 3.F }));
		// Direction: R1 * R2 * R3 applied to (1, 2, 3).
		const auto expectedDir = r1.rotated(r2.rotated(r3.rotated(math::fvec3 { 1.F, 2.F, 3.F })));

		const auto universe = child.getUniverseTransform();
		const auto loc = universe.location.into();
		EXPECT_NEAR(loc.x, expectedLoc.x, 1e-3F);
		EXPECT_NEAR(loc.y, expectedLoc.y, 1e-3F);
		EXPECT_NEAR(loc.z, expectedLoc.z, 1e-3F);

		const auto dir = universe.matrix * math::fvec4 { 1.F, 2.F, 3.F, 0.F };
		EXPECT_NEAR(dir.x, expectedDir.x, 1e-3F);
		EXPECT_NEAR(dir.y, expectedDir.y, 1e-3F);
		EXPECT_NEAR(dir.z, expectedDir.z, 1e-3F);

		const auto origin = universe.matrix * math::fvec4 { 0.F, 0.F, 0.F, 1.F };
		EXPECT_NEAR(origin.x, expectedLoc.x, 1e-3F);
		EXPECT_NEAR(origin.y, expectedLoc.y, 1e-3F);
		EXPECT_NEAR(origin.z, expectedLoc.z, 1e-3F);

		// The individual getters are views onto the same walk.
		EXPECT_EQ(child.getUniverseLocation(), universe.location);
	}

	TEST(HierarchyComponentTest, UniverseRotatorAccumulatesOverDeepChainBeyondInlineCapacity) {
		// 12 levels exceed the inline chain buffer (8), each adding 0.05 rad of yaw => 0.6 rad.
		auto chain = std::vector<HierarchyComponent> {};
		chain.reserve(12);
		for (auto i = 0; i < 12; ++i) {
			chain.push_back(make(i == 0 ? nullptr : &chain.back()));
			chain.back().getLocalTransform().setRotator(math::Rotator { 0.F, 0.05F, 0.F });
		}

		EXPECT_NEAR(chain.back().getUniverseRotator().yaw(), 0.6F, 1e-3F);
	}

	TEST(HierarchyComponentTest, UniverseTransformKeepsScaleOutOfTheRotationOverFiveLevels) {
		// Every level yaws by 0.1 rad and stretches x by 2; the stretch has to bend the matrix but never the rotator.
		auto chain = std::vector<HierarchyComponent> {};
		chain.reserve(5);
		for (auto i = 0; i < 5; ++i) {
			chain.push_back(make(i == 0 ? nullptr : &chain.back()));
			chain.back().getLocalTransform().setRotator(math::Rotator { 0.F, 0.1F, 0.F });
			chain.back().getLocalTransform().setScale(math::Vector3f32 { 2.F, 1.F, 1.F });
		}

		const auto universe = chain.back().getUniverseTransform();

		// Rotation: 5 * 0.1 = 0.5 rad of yaw, independent of the scale.
		EXPECT_NEAR(universe.rotator.pitch(), 0.F, 1e-3F);
		EXPECT_NEAR(universe.rotator.yaw(), 0.5F, 1e-3F);
		EXPECT_NEAR(universe.rotator.roll(), 0.F, 1e-3F);

		// Yaw turns around the y axis and the stretch only acts on x, so the y axis passes through all levels unchanged.
		const auto up = universe.matrix * math::fvec4 { 0.F, 1.F, 0.F, 0.F };
		EXPECT_NEAR(up.x, 0.F, 1e-3F);
		EXPECT_NEAR(up.y, 1.F, 1e-3F);
		EXPECT_NEAR(up.z, 0.F, 1e-3F);
	}

	TEST(HierarchyComponentTest, ChildSetterRoundTripsCombinedPoseThroughScaledRotatedParent) {
		auto root = make();
		root.getLocalTransform().setLocationAndRotator(math::Location { 4.F, -2.F, 1.F }, math::Rotator { 0.2F, 0.4F, -0.3F });
		root.getLocalTransform().setScale(math::Vector3f32 { 2.F, 1.F, 0.5F });
		auto child = make(&root);

		child.setRelativeLocationAndRotator(math::Location { 7.F, 3.F, -5.F }, math::Rotator { 0.1F, -0.5F, 0.6F });

		const auto universe = child.getUniverseTransform();
		const auto loc = universe.location.into();
		EXPECT_NEAR(loc.x, 7.F, 1e-3F);
		EXPECT_NEAR(loc.y, 3.F, 1e-3F);
		EXPECT_NEAR(loc.z, -5.F, 1e-3F);
		EXPECT_NEAR(universe.rotator.pitch(), 0.1F, 1e-3F);
		EXPECT_NEAR(universe.rotator.yaw(), -0.5F, 1e-3F);
		EXPECT_NEAR(universe.rotator.roll(), 0.6F, 1e-3F);
	}
}
