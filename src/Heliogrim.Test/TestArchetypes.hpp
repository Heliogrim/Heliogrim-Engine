#pragma once

#include <Engine.ACS/Archetype/Layout.hpp>
#include <Engine.ACS/Archetype/Overridable.hpp>
#include <Heliogrim/ActorInitializer.hpp>
#include <Heliogrim/Actor/Actor.hpp>
#include <Heliogrim/Actor/Archetype/InheritArchetype.hpp>
#include <Heliogrim/Component/HierarchyComponent.hpp>

namespace HeliogrimApi {
	using namespace ::hg;
	using namespace ::hg::engine::acs;

	/* Component stubs: plain HierarchyComponents with a payload to write through. */

	struct BodyStub : HierarchyComponent {
		using HierarchyComponent::HierarchyComponent;
		u64 mass = 0uLL;
	};

	struct WheelComponentStub : HierarchyComponent {
		using HierarchyComponent::HierarchyComponent;
		float radius = 0.F;
	};

	struct HubStub : HierarchyComponent {
		using HierarchyComponent::HierarchyComponent;
		u32 bolts = 0u;
	};

	struct CameraStub : HierarchyComponent {
		using HierarchyComponent::HierarchyComponent;
		u32 ticks = 0u;
	};

	// Larger than CameraStub, so overriding "camera" with it moves base slots.
	struct BigCameraStub : CameraStub {
		using CameraStub::CameraStub;
		u64 zoom[8] {};
	};

	struct TurretStub : HierarchyComponent {
		using HierarchyComponent::HierarchyComponent;
		u32 shells = 0u;
	};

	class TestBody :
		public InheritArchetype<TestBody, Actor> {
	public:
		using this_type = TestBody;
		constexpr static type_id typeId { "TestBody"_typeId };

		using layout = ActorLayout<
			Slot<"body", BodyStub, Children<
				Slot<"wheels", WheelComponentStub, Count<4>, Children<
					Slot<"hub", HubStub>>>,
				Slot<"camera", CameraStub>>>>;

	public:
		explicit TestBody(cref<ActorInitializer> initializer_);

		// Plain (non-template) body in the .cpp; stays correct for derived archetypes with append-only layouts.
		void touchCamera();
	};

	class TestTank :
		public InheritArchetype<TestTank, TestBody> {
	public:
		using this_type = TestTank;
		constexpr static type_id typeId { "TestTank"_typeId };

		using layout = Extend<TestBody::layout, Slot<"turret", TurretStub>>;

	public:
		explicit TestTank(cref<ActorInitializer> initializer_);

	public:
		u64 armor[5] {};
	};

	class TestTicker :
		public InheritArchetype<TestTicker, Actor> {
	public:
		using this_type = TestTicker;
		constexpr static type_id typeId { "TestTicker"_typeId };

		using layout = ActorLayout<Slot<"body", BodyStub, Children<Slot<"camera", CameraStub>>>>;

	public:
		explicit TestTicker(cref<ActorInitializer> initializer_);

		// Plain body in the .cpp; inherited by TestTickerChild, which declares no tick.
		void tick();
	};

	class TestTickerChild :
		public InheritArchetype<TestTickerChild, TestTicker> {
	public:
		using this_type = TestTickerChild;
		constexpr static type_id typeId { "TestTickerChild"_typeId };

		using layout = Extend<TestTicker::layout, Slot<"turret", TurretStub>>;

	public:
		explicit TestTickerChild(cref<ActorInitializer> initializer_);
	};

	class TestOverridable :
		public InheritArchetype<TestOverridable, Actor> {
	public:
		using this_type = TestOverridable;
		constexpr static type_id typeId { "TestOverridable"_typeId };

		using layout = Overridable<ActorLayout<
			Slot<"body", BodyStub, Children<
				Slot<"camera", CameraStub>,
				Slot<"turret", TurretStub>>>>>;

	public:
		explicit TestOverridable(cref<ActorInitializer> initializer_);

		// Body in TestArchetypes.inl.hpp.
		void tick(this auto& self);
	};

	class TestOverrider :
		public InheritArchetype<TestOverrider, TestOverridable> {
	public:
		using this_type = TestOverrider;
		constexpr static type_id typeId { "TestOverrider"_typeId };

		using layout = Extend<TestOverridable::layout, Override<"camera", BigCameraStub>>;

	public:
		explicit TestOverrider(cref<ActorInitializer> initializer_);

		// Body in TestArchetypes.inl.hpp; forwards to TestOverridable::tick.
		void tick(this auto& self);
	};
}
