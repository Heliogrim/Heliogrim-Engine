#include "pch.h"

/**/

#include <memory>
#include <type_traits>
#include <unordered_set>
#include <vector>
#include <Engine.ACS/ActorModule.hpp>
#include <Engine.ACS/Registry.hpp>
#include <Engine.Common/Make.hpp>
#include <Engine.Common/Move.hpp>
#include <Engine.Config/Config.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Core/Timing.hpp>
#include <Engine.Core/Universe.hpp>
#include <Engine.Core/Module/Modules.hpp>
#include <Engine.Event/GlobalEventEmitter.hpp>
#include <Engine.Level/Level.hpp>
#include <Engine.Scene/SceneBase.hpp>
#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>
#include <Heliogrim/IComponentRegisterContext.hpp>
#include <Heliogrim/Actor/ActorRef.hpp>
#include <Heliogrim/Actor/Archetype/ArchetypeConcept.hpp>
#include <Heliogrim/Actor/Archetype/ArchetypeDefinition.inl.hpp>
#include <Heliogrim/Actor/Geometry/BoxShapeGeometryActor.hpp>
#include <Heliogrim/Actor/Geometry/SphereShapeGeometryActor.hpp>
#include <Heliogrim/Actor/Geometry/StaticGeometryActor.hpp>
#include <Heliogrim/Component/Scene/Collision/BoxShapeCollisionComponent.Layout.hpp>
#include <Heliogrim/Component/Scene/Collision/SphereShapeCollisionComponent.Layout.hpp>
#include <Heliogrim/Component/Scene/Geometry/BoxShapeGeometryComponent.Layout.hpp>
#include <Heliogrim/Component/Scene/Geometry/SphereShapeGeometryComponent.Layout.hpp>
#include <Heliogrim/Component/Scene/Geometry/StaticGeometryComponent.Layout.hpp>
#include <Heliogrim/Universe/Universe.hpp>

#include "SpawnEngine.hpp"
#include "TestArchetypes.hpp"

using namespace HeliogrimApi;

// gtest TEST bodies declare a member function TestBody(), which hides the fixture archetype of that name.
using Body = HeliogrimApi::TestBody;

namespace HeliogrimApi::spawn_fixture {
	/* Test-local archetypes: a destructor-counting component and a class that reuses TestBody's type id. */

	u32 countedDestroyed = 0u;

	struct CountedStub : HierarchyComponent {
		using HierarchyComponent::HierarchyComponent;

		~CountedStub() override {
			++countedDestroyed;
		}
	};

	class CountedActor :
		public InheritArchetype<CountedActor, Actor> {
	public:
		using this_type = CountedActor;
		constexpr static type_id typeId { "SpawnCountedActor"_typeId };

		using layout = ActorLayout<Slot<"root", CountedStub, Children<Slot<"leaf", CountedStub, Count<3>>>>>;

	public:
		explicit CountedActor(cref<ActorInitializer> initializer_) :
			InheritArchetype(initializer_) {}
	};

	class ShadowBody :
		public InheritArchetype<ShadowBody, Actor> {
	public:
		using this_type = ShadowBody;
		constexpr static type_id typeId { "TestBody"_typeId };

		using layout = ActorLayout<Slot<"body", BodyStub>>;

	public:
		explicit ShadowBody(cref<ActorInitializer> initializer_) :
			InheritArchetype(initializer_) {}
	};
}

// CountedStub carries no payload; the Layout exists so the archetype can generate its dump.
template <>
class hg::engine::serialization::access::Layout<HeliogrimApi::spawn_fixture::CountedStub> {
public:
	constexpr static ::hg::u64 version = 1uLL;
	constexpr static ::hg::u64 size = 0uLL;

public:
	static void dump(::hg::ref<const HeliogrimApi::spawn_fixture::CountedStub>, ::hg::ref<::hg::Vector<std::byte>>) {}

	static void load(std::span<const std::byte>, ::hg::ref<HeliogrimApi::spawn_fixture::CountedStub>) {}
};

template <>
void hg::engine::serialization::access::Structure<HeliogrimApi::spawn_fixture::CountedStub>::serialize(
	::hg::ref<const HeliogrimApi::spawn_fixture::CountedStub>,
	::hg::mref<StructScopedSlot>
) {}

template <>
void hg::engine::serialization::access::Structure<HeliogrimApi::spawn_fixture::CountedStub>::hydrate(
	::hg::ref<const StructScopedSlot>,
	::hg::ref<HeliogrimApi::spawn_fixture::CountedStub>
) {}

template class hg::InheritArchetype<HeliogrimApi::spawn_fixture::CountedActor, hg::Actor>;
template class hg::InheritArchetype<HeliogrimApi::spawn_fixture::ShadowBody, hg::Actor>;

namespace HeliogrimApi {
	namespace {
		using namespace spawn_fixture;

		class RecordingContext final :
			public IComponentRegisterContext {
		public:
			std::unordered_set<ptr<HierarchyComponent>> live {};
			size_t adds = 0uLL;

			void add(const ptr<HierarchyComponent> component_) override {
				++adds;
				live.insert(component_);
			}

			void add(ref<const ComponentHierarchy>) override {}

			void remove(const ptr<HierarchyComponent> component_) override {
				live.erase(component_);
			}

			void remove(ref<const ComponentHierarchy>) override {}
		};

		class MockScene final :
			public engine::scene::SceneBase {
		public:
			RecordingContext context {};

			void prepare() override {}

			void cleanup() override {}

			void broadcast(engine::scene::SceneBroadcastFlags) override {}

			void update(engine::scene::SceneUpdateFlags) override {}

			void postprocess(engine::scene::ScenePostProcessFlags) override {}

			bool add(const ptr<SceneComponent>) override {
				return false;
			}

			bool remove(const ptr<const SceneComponent>) override {
				return false;
			}

			[[nodiscard]] nmpt<IComponentRegisterContext> registerContext() noexcept override {
				return &context;
			}

			[[nodiscard]] bool hasSceneSystem(const ptr<const MetaClass>) const noexcept override {
				return false;
			}

			[[nodiscard]] nmpt<engine::scene::SceneSystemBase> getSceneSystem(size_t) const noexcept override {
				return nullptr;
			}

			[[nodiscard]] nmpt<engine::scene::SceneSystemBase> getSceneSystem(const ptr<const MetaClass>) const noexcept override {
				return nullptr;
			}
		};

		using test_engine::SpawnEngine;

		template <typename Type_>
		[[nodiscard]] ref<Type_> as(const ptr<Actor> actor_) noexcept {
			return *static_cast<ptr<Type_>>(actor_);
		}

		void noInit(ref<Actor>, size_t) noexcept {}

		[[nodiscard]] ptr<Actor> deref(cref<ActorRef<>> ref_) noexcept {
			auto actor = ref_.get();
			return actor.has_value() ? std::addressof(actor.value()) : nullptr;
		}

		template <typename Left_, typename Right_>
		concept CrossComparable = requires(cref<Left_> left_, cref<Right_> right_) {
			left_ == right_;
		};

		static_assert(not std::is_convertible_v<ActorGuid, ComponentGuid>);
		static_assert(not std::is_convertible_v<ComponentGuid, ActorGuid>);
		static_assert(not std::is_convertible_v<Guid, ActorGuid>);
		static_assert(not std::is_convertible_v<Guid, ComponentGuid>);
		static_assert(not std::is_convertible_v<u64, engine::acs::Handle>);
		static_assert(not std::is_convertible_v<engine::acs::Handle, u64>);
		template <typename Left_, typename Right_>
		concept CrossOrdered = requires(cref<Left_> left_, cref<Right_> right_) {
			left_ <=> right_;
		};

		static_assert(not std::equality_comparable_with<ActorGuid, Guid>);
		static_assert(not std::equality_comparable_with<ComponentGuid, Guid>);
		static_assert(not std::equality_comparable_with<ActorGuid, ComponentGuid>);
		static_assert(not std::three_way_comparable_with<ActorGuid, Guid>);
		static_assert(not std::three_way_comparable_with<ComponentGuid, Guid>);
		static_assert(not std::three_way_comparable_with<ActorGuid, ComponentGuid>);
		static_assert(not CrossComparable<ActorGuid, Guid>);
		static_assert(not CrossComparable<Guid, ActorGuid>);
		static_assert(not CrossComparable<ComponentGuid, Guid>);
		static_assert(not CrossOrdered<ActorGuid, ComponentGuid>);
		static_assert(not CrossOrdered<ComponentGuid, ActorGuid>);
		static_assert(not CrossOrdered<ActorGuid, Guid>);
		static_assert(not CrossOrdered<ComponentGuid, Guid>);
		static_assert(CrossOrdered<ActorGuid, ActorGuid>);
		static_assert(std::three_way_comparable<ActorGuid>);
		static_assert(std::three_way_comparable<ComponentGuid>);
		static_assert(not std::is_convertible_v<u16, engine::acs::SpaceId>);
		static_assert(not std::is_constructible_v<engine::acs::Handle, u64>);
		static_assert(
			std::is_invocable_v<
				decltype(&engine::acs::Handle::make), engine::acs::SpaceId, engine::acs::HandleGeneration, engine::acs::HandleIndex
			>
		);
		static_assert(
			not std::is_invocable_v<
				decltype(&engine::acs::Handle::make), engine::acs::SpaceId, engine::acs::HandleIndex, engine::acs::HandleGeneration
			>
		);
		static_assert(not std::is_invocable_v<decltype(&engine::acs::Handle::make), u16, u32, u32>);
		static_assert(not std::is_constructible_v<ActorRef<>, engine::acs::Handle>);
		static_assert(not CrossComparable<ActorGuid, ComponentGuid>);
		static_assert(not CrossComparable<ComponentGuid, ActorGuid>);
		static_assert(CrossComparable<ActorGuid, ActorGuid>);
	}

	class Spawn :
		public ::testing::Test {
	protected:
		void SetUp() override {
			_engine = make_uptr<SpawnEngine>();

			auto scene = make_uptr<MockScene>();
			_context = &scene->context;

			auto core = make_sptr<engine::core::Universe>(
				::hg::move(scene),
				DenseSet<Arci<engine::core::Level>> { engine::core::make_root_like_level() }
			);
			core->commitLevel(*core->getRootLevel());
			_universe = Universe { ::hg::move(core) };
		}

		void TearDown() override {
			// The universe owns the levels, whose destructors reach the engine's registry.
			_universe = Universe {};
			_engine.reset();
		}

		uptr<SpawnEngine> _engine;
		ptr<RecordingContext> _context = nullptr;
		Universe _universe;

		[[nodiscard]] ref<engine::acs::Registry> registry() const noexcept {
			return *_engine->getActors()->getRegistry();
		}

		[[nodiscard]] ref<engine::core::Level> rootLevel() const noexcept {
			return *_universe.unwrap()->getRootLevel();
		}
	};

	TEST_F(Spawn, LookupNeedsRegistration) {
		EXPECT_FALSE(FindActorClass(Body::typeId).has_value());

		EXPECT_TRUE(RegisterActorClass(&Body::actorClass()));
		EXPECT_TRUE(RegisterActorClass<Body>());

		const auto found = FindActorClass(Body::typeId);
		ASSERT_TRUE(found.has_value());
		EXPECT_EQ(&*found, static_cast<ptr<const ActorClass>>(&Body::actorClass()));

		// Same type id, different class.
		EXPECT_FALSE(RegisterActorClass<ShadowBody>());
		EXPECT_EQ(&*FindActorClass(Body::typeId), static_cast<ptr<const ActorClass>>(&Body::actorClass()));
		EXPECT_FALSE(RegisterActorClass(nullptr));
	}

	TEST_F(Spawn, CreateTemplatePathWithoutLookup) {
		auto actor = CreateActor<Body>();
		ASSERT_NE(actor.get(), nullptr);
		EXPECT_FALSE(FindActorClass(Body::typeId).has_value());

		EXPECT_EQ(actor->getArchetype(), static_cast<ptr<const ActorClass>>(&Body::actorClass()));
		EXPECT_EQ(actor->getRootComponent().address(), &actor->get<"body">());

		const auto handle = registry().handleOf(*actor);
		EXPECT_EQ(handle.space(), engine::acs::detachedSpace);
		EXPECT_EQ(registry().resolve(handle), static_cast<ptr<Actor>>(actor.get()));
	}

	TEST_F(Spawn, GeometryActorsAreArchetypes) {
		static_assert(IsArchetype<StaticGeometryActor>);
		static_assert(IsArchetype<BoxShapeGeometryActor>);
		static_assert(IsArchetype<SphereShapeGeometryActor>);

		auto geometry = CreateActor<StaticGeometryActor>();
		ASSERT_NE(geometry.get(), nullptr);
		EXPECT_EQ(geometry->getRootComponent().address(), &geometry->get<"geometry">());

		auto box = CreateActor<BoxShapeGeometryActor>();
		ASSERT_NE(box.get(), nullptr);
		EXPECT_EQ(box->getRootComponent().address(), &box->get<"collision">());
		EXPECT_EQ(box->get<"geometry">().getParentComponent(), &box->get<"collision">());

		auto sphere = CreateActor<SphereShapeGeometryActor>();
		ASSERT_NE(sphere.get(), nullptr);
		EXPECT_EQ(sphere->getRootComponent().address(), &sphere->get<"collision">());
		EXPECT_EQ(sphere->get<"geometry">().getParentComponent(), &sphere->get<"collision">());
	}

	TEST_F(Spawn, Batch10k) {
		constexpr size_t count = 10'000uLL;
		const auto entries = Body::actorClass().componentOffsets().size();

		auto actors = hg::await(
			SpawnActors(
				&Body::actorClass(),
				count,
				[](ref<Actor> actor_, const size_t index_) {
					static_cast<ref<Body>>(actor_).get<"body">().mass = index_;
				},
				_universe
			)
		).value();
		ASSERT_EQ(actors.size(), count);
		EXPECT_EQ(rootLevel().getActors().size(), count);

		const auto space = rootLevel().space();
		EXPECT_NE(space, engine::acs::detachedSpace);

		std::unordered_set<Guid> guids {};
		for (size_t index = 0uLL; index < count; ++index) {
			auto* const actor = deref(actors[index]);
			ASSERT_NE(actor, nullptr);
			const auto handle = registry().handleOf(*actor);
			ASSERT_NE(handle, engine::acs::Handle {});
			EXPECT_EQ(actors[index].handle(), handle);
			EXPECT_EQ(handle.space(), space);
			EXPECT_EQ(registry().resolve(handle), actor);
			EXPECT_EQ(as<Body>(actor).get<"body">().mass, index);
			EXPECT_EQ(actor->getLevel().address(), &rootLevel());
			guids.insert(actor->guid());

			for (auto* const component : actor->getComponents()) {
				EXPECT_TRUE(_context->live.contains(component));
			}
		}
		EXPECT_EQ(guids.size(), count);
		EXPECT_EQ(_context->adds, count * entries);
	}

	TEST_F(Spawn, DetachedThenAddPreMountHandleStale) {
		auto created = CreateActor<Body>();
		auto* const before = static_cast<ptr<Actor>>(created.get());
		const auto detached = registry().handleOf(*before);
		ASSERT_EQ(detached.space(), engine::acs::detachedSpace);
		created->get<"body">().mass = 7uLL;

		const auto added = _universe.addActor(VolatileActor<> { ::hg::move(created) });
		ASSERT_TRUE(added.has_value());
		auto* const after = deref(added.value());

		EXPECT_NE(after, before);
		const auto mounted = registry().handleOf(*after);
		EXPECT_EQ(added.value().handle(), mounted);
		EXPECT_EQ(mounted.space(), rootLevel().space());
		EXPECT_EQ(registry().resolve(mounted), after);
		// Intended: a handle taken before addActor goes stale on mount (handles belong to their space) and never aliases
		// another actor; the new handle from the level space resolves.
		EXPECT_EQ(registry().resolve(detached), nullptr);

		auto& body = as<Body>(after);
		EXPECT_EQ(body.get<"body">().mass, 7uLL);
		EXPECT_EQ(body.get<"wheels">()[2]->getOwner(), after);
		EXPECT_EQ(body.getRootComponent().address(), &body.get<"body">());
		EXPECT_TRUE(_context->live.contains(&body.get<"camera">()));
		EXPECT_EQ(rootLevel().getActors().size(), 1uLL);
	}

	TEST_F(Spawn, DetachedThenAddKeepsLabel) {
		// Longer than any small-string buffer, so the text lives on the heap and must be re-seated, not byte-copied.
		const String text = "a detached archetype actor with a heap-allocated label";

		auto created = CreateActor<Body>();
		created->setLabel(ActorLabel { text });
		auto* const before = static_cast<ptr<Actor>>(created.get());

		const auto added = _universe.addActor(VolatileActor<> { ::hg::move(created) });
		ASSERT_TRUE(added.has_value());
		auto* const after = deref(added.value());
		ASSERT_NE(after, nullptr);
		ASSERT_NE(after, before);

		const auto& moved = after->label().text;
		EXPECT_EQ(moved, text);
		String copy {};
		for (const char c : moved) {
			copy.push_back(c);
		}
		EXPECT_EQ(copy, text);

		// The moved label stays usable: write through it and rewrite it.
		after->setLabel(ActorLabel { moved + "!" });
		EXPECT_EQ(after->label().text, text + "!");
	}

	TEST_F(Spawn, DestroyReusesSlotStaleRefNone) {
		auto keeper = CreateActor<Body>();
		auto victim = CreateActor<Body>();
		auto* const slot = static_cast<ptr<Actor>>(victim.get());
		const auto stale = registry().handleOf(*slot);

		EXPECT_TRUE(Destroy(VolatileActor<> { ::hg::move(victim) }));
		EXPECT_EQ(registry().resolve(stale), nullptr);

		// The slot is freed at housekeeping, not before.
		auto early = CreateActor<Body>();
		EXPECT_NE(static_cast<ptr<Actor>>(early.get()), slot);

		registry().housekeeping().commit();

		auto reused = CreateActor<Body>();
		EXPECT_EQ(static_cast<ptr<Actor>>(reused.get()), slot);

		const auto fresh = registry().handleOf(*reused);
		EXPECT_NE(fresh, stale);
		EXPECT_EQ(registry().resolve(fresh), static_cast<ptr<Actor>>(reused.get()));
		EXPECT_EQ(registry().resolve(stale), nullptr);
	}

	TEST_F(Spawn, LevelDestroyFreesAll) {
		constexpr size_t count = 50uLL;
		const auto& actorClass = CountedActor::actorClass();
		const auto perActor = static_cast<u32>(actorClass.componentOffsets().size());
		countedDestroyed = 0u;

		auto internal = Arci<engine::core::Level>::create(generate_level_guid());

		auto actors = hg::await(SpawnActors(&actorClass, count, noInit, _universe, Some(*internal))).value();
		ASSERT_EQ(actors.size(), count);

		const auto space = internal->space();
		ASSERT_NE(space, engine::acs::detachedSpace);
		ASSERT_NE(registry().region(space, &actorClass), nullptr);
		const auto handle = actors.front().handle();
		EXPECT_EQ(registry().handleOf(*deref(actors.front())), handle);
		EXPECT_EQ(countedDestroyed, 0u);

		internal.reset();

		EXPECT_EQ(countedDestroyed, count * perActor);
		EXPECT_EQ(registry().region(space, &actorClass), nullptr);
		EXPECT_FALSE(registry().hasSpace(space));
		EXPECT_EQ(registry().resolve(handle), nullptr);
		// Fresh refs never resolved before, so they re-resolve and see the freed handles.
		ActorRef<> fresh {};
		archetype_detail::RefAccess::handle(fresh) = handle;
		EXPECT_FALSE(fresh.get().has_value());

		// Destructors ran exactly once.
		registry().housekeeping().commit();
		EXPECT_EQ(countedDestroyed, count * perActor);
	}

	TEST_F(Spawn, LegacyActorAddsWithoutRef) {
		auto created = CreateActor();
		auto* const raw = created.get();
		ASSERT_NE(raw, nullptr);
		// Contract: Some exactly for archetype actors; a legacy actor has no handle, so None, but it is still mounted.
		EXPECT_FALSE(ActorRef<>::of(raw).has_value());

		const auto added = _universe.addActor(VolatileActor<> { ::hg::move(created) });
		EXPECT_FALSE(added.has_value());
		EXPECT_EQ(rootLevel().getActors().size(), 1uLL);
		EXPECT_EQ(raw->getLevel().address(), &rootLevel());
	}

	TEST_F(Spawn, RefFailurePathsAreEmpty) {
		EXPECT_FALSE(ActorRef<>::of(nullptr).has_value());
		EXPECT_FALSE(ActorRef<> {}.get().has_value());

		// A detached actor has a handle, but its ref stays empty until mounted.
		auto created = CreateActor<Body>();
		const auto detached = ActorRef<>::of(created.get());
		ASSERT_TRUE(detached.has_value());
		EXPECT_FALSE(detached.value().get().has_value());

		const auto spawned = hg::await(SpawnActor(&Body::actorClass(), _universe)).value();
		ASSERT_TRUE(spawned.has_value());
		EXPECT_NE(deref(spawned.value()), nullptr);
	}

	TEST_F(Spawn, SpansPerType) {
		constexpr size_t count = 100uLL;
		const auto stride = Body::actorClass().desc().stride;
		static_cast<void>(hg::await(SpawnActors(&Body::actorClass(), count, noInit, _universe)).value());

		size_t wheels = 0uLL;
		registry().forEachSpan<WheelComponentStub>(
			[&](const engine::acs::ComponentSpan<WheelComponentStub> span_) {
				EXPECT_EQ(span_.stride, stride);
				for (u32 index = 0u; index < span_.count; ++index) {
					EXPECT_NE(span_[index].getOwner(), nullptr);
				}
				wheels += span_.count;
			}
		);
		EXPECT_EQ(wheels, count * 4uLL);

		size_t cameras = 0uLL;
		registry().forEachSpan<CameraStub>([&](const engine::acs::ComponentSpan<CameraStub> span_) { cameras += span_.count; });
		EXPECT_EQ(cameras, count);

		// Derived types count for their base: every entry is a HierarchyComponent.
		size_t all = 0uLL;
		registry().forEachSpan<HierarchyComponent>(
			[&](const engine::acs::ComponentSpan<HierarchyComponent> span_) { all += span_.count; }
		);
		EXPECT_EQ(all, count * Body::actorClass().componentOffsets().size());
	}
}
