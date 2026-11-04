#include "pch.h"

#include <cstddef>
#include <new>
#include <type_traits>
#include <Engine.ACS/Registry.hpp>
#include <Engine.ACS/Archetype/LayoutInfo.hpp>
#include <Heliogrim/Actor/Archetype/ActorClass.hpp>
#include <Heliogrim/Actor/Archetype/ArchetypeConcept.hpp>

#include "TestArchetypes.hpp"

using namespace HeliogrimApi;

// gtest TEST bodies declare a member function TestBody(), which hides the fixture archetype of that name.
using Body = HeliogrimApi::TestBody;

namespace HeliogrimApi {
	namespace {
		/* Slot memory of `count_` archetype actors, constructed and destroyed through the class ops. */
		class SlotSpan {
		public:
			SlotSpan(cref<ActorClass> class_, const u32 count_, const bool construct_ = true) :
				_class(class_),
				_count(count_),
				_memory(
					static_cast<std::byte*>(
						::operator new(static_cast<size_t>(count_) * class_.desc().stride, std::align_val_t { class_.desc().align })
					)
				),
				_live(construct_) {
				if (_live) {
					_class.ops().construct(_memory, _count, stride(), ArchetypeInit { registry() });
				}
			}

			SlotSpan(const SlotSpan&) = delete;

			SlotSpan& operator=(const SlotSpan&) = delete;

			~SlotSpan() {
				if (_live) {
					_class.ops().destroy(_memory, _count, stride());
				}
				::operator delete(_memory, std::align_val_t { _class.desc().align });
			}

		private:
			cref<ActorClass> _class;
			u32 _count;
			std::byte* _memory;
			bool _live;

		public:
			[[nodiscard]] static ref<engine::acs::Registry> registry() {
				static engine::acs::Registry instance {};
				return instance;
			}

			[[nodiscard]] u32 stride() const noexcept {
				return _class.desc().stride;
			}

			[[nodiscard]] std::byte* slot(const u32 index_) const noexcept {
				return _memory + static_cast<size_t>(index_) * stride();
			}

			template <typename Type_>
			[[nodiscard]] ref<Type_> actor(const u32 index_) const noexcept {
				return *std::launder(reinterpret_cast<Type_*>(slot(index_) + _class.desc().actorOffset));
			}

			// Takes over live slots moved in by `relocate`.
			void adopt(SlotSpan& from_) noexcept {
				_class.ops().relocate(from_._memory, _memory, _count, stride());
				from_._live = false;
				_live = true;
			}
		};

		template <typename Type_>
		using info_t = LayoutInfo<typename Type_::layout, Type_>;

		[[nodiscard]] const void* address(const auto& object_) noexcept {
			return static_cast<const void*>(std::addressof(object_));
		}
	}

	TEST(Archetype, ActorClassTyped) {
		static_assert(IsArchetype<Body>);
		static_assert(IsArchetype<TestTank>);
		static_assert(IsArchetype<TestOverrider>);
		static_assert(not IsArchetype<Actor>);
		static_assert(std::is_same_v<decltype(TestTank::actorClass()), const TypedActorClass<TestTank>&>);

		EXPECT_NE(
			static_cast<const ActorClass*>(&TestTank::actorClass()),
			static_cast<const ActorClass*>(&Body::actorClass())
		);
		EXPECT_EQ(TestTank::actorClass().metaClass(), static_cast<ptr<const MetaClass>>(TestTank::getStaticMetaClass()));
		EXPECT_EQ(TestTank::actorClass().desc().stride, info_t<TestTank>::slot_stride);
		EXPECT_EQ(TestTank::actorClass().componentOffsets().size(), info_t<TestTank>::entry_count);
		EXPECT_NE(TestTank::actorClass().layoutHash(), Body::actorClass().layoutHash());
	}

	TEST(Archetype, GetResolves) {
		const auto& actorClass = Body::actorClass();
		SlotSpan span { actorClass, 2u };
		auto& actor = span.actor<Body>(1u);

		constexpr u32 camera = info_t<Body>::index_of<"camera">();
		static_assert(std::is_same_v<decltype(actor.get<"camera">()), CameraStub&>);
		EXPECT_EQ(address(actor.get<"camera">()), span.slot(1u) + actorClass.componentOffsets()[camera]);

		auto wheels = actor.get<"wheels">();
		static_assert(decltype(wheels)::size() == 4uLL);
		auto& hub = wheels[2].get<"hub">();
		static_assert(std::is_same_v<decltype(hub), HubStub&>);
		EXPECT_EQ(hub.getParentComponent(), &*wheels[2]);
		EXPECT_NE(address(*wheels[1]), address(*wheels[2]));
		EXPECT_EQ(wheels[2]->getParentComponent(), &actor.get<"body">());

		EXPECT_EQ(hub.getOwner(), &actor);
		EXPECT_EQ(actor.getRootComponent().address(), &actor.get<"body">());

		const auto components = actor.getComponents();
		EXPECT_EQ(components.size(), info_t<Body>::entry_count);
		EXPECT_TRUE(components.contains(&hub));
		EXPECT_FALSE(components.contains(&span.actor<Body>(0u).get<"camera">()));
	}

	TEST(Archetype, PlainBaseBodyOnDerived) {
		SlotSpan span { TestTank::actorClass(), 1u };
		auto& tank = span.actor<TestTank>(0u);

		// Non-template Body::touchCamera resolves "camera" against Body's layout, at the same actor distance.
		tank.touchCamera();

		EXPECT_EQ(tank.get<"camera">().ticks, 1u);
		EXPECT_EQ(address(static_cast<Body&>(tank).get<"camera">()), address(tank.get<"camera">()));
		EXPECT_EQ(tank.get<"turret">().getParentComponent(), &tank.get<"body">());
	}

	TEST(Archetype, TickHookDetected) {
		static_assert(not has_tick_v<Body>);
		static_assert(not has_tick_v<TestTank>);
		static_assert(has_tick_v<TestOverridable>);
		static_assert(has_tick_v<TestOverrider>);

		EXPECT_EQ(Body::actorClass().ops().tick, nullptr);
		EXPECT_EQ(TestTank::actorClass().ops().tick, nullptr);
		EXPECT_NE(TestOverridable::actorClass().ops().tick, nullptr);
		EXPECT_NE(TestOverrider::actorClass().ops().tick, nullptr);
	}

	TEST(Archetype, DeducingThisOverride) {
		using base = info_t<TestOverridable>;
		using derived = info_t<TestOverrider>;
		// The override must move "camera", or this test proves nothing about resolving against the concrete layout.
		static_assert(base::actor_relative(base::index_of<"camera">()) != derived::actor_relative(derived::index_of<"camera">()));

		const auto& actorClass = TestOverrider::actorClass();
		SlotSpan span { actorClass, 3u };
		actorClass.ops().tick(span.slot(0u), 3u, span.stride());

		for (u32 index = 0u; index < 3u; ++index) {
			auto& actor = span.actor<TestOverrider>(index);
			auto& camera = actor.get<"camera">();
			static_assert(std::is_same_v<decltype(camera), BigCameraStub&>);

			EXPECT_EQ(address(camera), span.slot(index) + derived::entries[derived::index_of<"camera">()].offset);
			EXPECT_EQ(camera.ticks, 1u);
			EXPECT_EQ(camera.zoom[0], 42uLL);
		}
	}

	TEST(Archetype, RelocateRestampsLinks) {
		const auto& actorClass = TestTank::actorClass();
		SlotSpan from { actorClass, 2u };
		SlotSpan to { actorClass, 2u, false };
		const auto guid = from.actor<TestTank>(1u).guid();
		from.actor<TestTank>(1u).get<"wheels">()[3]->radius = 7.F;

		to.adopt(from);

		auto& actor = to.actor<TestTank>(1u);
		auto wheels = actor.get<"wheels">();
		EXPECT_EQ(actor.guid(), guid);
		EXPECT_EQ(wheels[3]->radius, 7.F);
		EXPECT_EQ(wheels[3].get<"hub">().getParentComponent(), &*wheels[3]);
		EXPECT_EQ(wheels[3].get<"hub">().getOwner(), &actor);
		EXPECT_EQ(actor.get<"body">().getParentComponent(), nullptr);
		EXPECT_EQ(actor.getRootComponent().address(), &actor.get<"body">());
		EXPECT_TRUE(actor.getComponents().contains(&actor.get<"turret">()));
	}

	TEST(Archetype, AddComponentAsserts) {
		SlotSpan span { Body::actorClass(), 1u };
		auto& actor = span.actor<Body>(0u);

		EXPECT_DEATH(
			actor.addComponent(
				VolatileComponent<HierarchyComponent> {
					new HierarchyComponent { generate_component_guid(), CachedActorPointer { actor.guid(), &actor }, nullptr }
				},
				None
			),
			""
		);
	}
}
