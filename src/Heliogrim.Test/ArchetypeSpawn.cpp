#include "pch.h"

#include <unordered_set>
#include <vector>
#include <Engine.ACS/ActorModule.hpp>
#include <Engine.ACS/Registry.hpp>
#include <Engine.Common/Make.hpp>
#include <Engine.Config/Config.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Core/Timing.hpp>
#include <Engine.Core/Universe.hpp>
#include <Engine.Core/Module/Modules.hpp>
#include <Engine.Event/GlobalEventEmitter.hpp>
#include <Engine.Level/Level.hpp>
#include <Engine.Scene/SceneBase.hpp>
#include <Heliogrim/IComponentRegisterContext.hpp>
#include <Heliogrim/Actor/Archetype/ArchetypeDefinition.inl.hpp>
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
	}

	class Spawn :
		public ::testing::Test {
	protected:
		void SetUp() override {
			_engine = make_uptr<SpawnEngine>();

			auto scene = make_uptr<MockScene>();
			_context = &scene->context;

			auto core = make_sptr<engine::core::Universe>(
				std::move(scene),
				DenseSet<Arci<engine::core::Level>> { engine::core::make_root_like_level() }
			);
			core->commitLevel(*core->getRootLevel());
			_universe = Universe { std::move(core) };
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
		EXPECT_EQ(*found, static_cast<ptr<const ActorClass>>(&Body::actorClass()));

		// Same type id, different class.
		EXPECT_FALSE(RegisterActorClass<ShadowBody>());
		EXPECT_EQ(*FindActorClass(Body::typeId), static_cast<ptr<const ActorClass>>(&Body::actorClass()));
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

	TEST_F(Spawn, Batch10k) {
		constexpr size_t count = 10'000uLL;
		const auto entries = Body::actorClass().componentOffsets().size();

		auto actors = SpawnActors(
			&Body::actorClass(),
			count,
			[](ref<Actor> actor_, const size_t index_) {
				static_cast<ref<Body>>(actor_).get<"body">().mass = index_;
			},
			_universe
		).get();
		ASSERT_EQ(actors.size(), count);
		EXPECT_EQ(rootLevel().getActors().size(), count);

		const auto space = rootLevel().space();
		EXPECT_NE(space, engine::acs::detachedSpace);

		std::unordered_set<Guid> guids {};
		for (size_t index = 0uLL; index < count; ++index) {
			auto* const actor = actors[index];
			const auto handle = registry().handleOf(*actor);
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

		auto* const after = _universe.addActor(VolatileActor<> { std::move(created) });

		EXPECT_NE(after, before);
		const auto mounted = registry().handleOf(*after);
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

		auto* const after = _universe.addActor(VolatileActor<> { std::move(created) });
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

		EXPECT_TRUE(Destroy(VolatileActor<> { std::move(victim) }));
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

		auto actors = SpawnActors(&actorClass, count, noInit, _universe, Some(*internal)).get();
		ASSERT_EQ(actors.size(), count);

		const auto space = internal->space();
		ASSERT_NE(space, engine::acs::detachedSpace);
		ASSERT_NE(registry().region(space, &actorClass), nullptr);
		const auto handle = registry().handleOf(*actors.front());
		EXPECT_EQ(countedDestroyed, 0u);

		internal.reset();

		EXPECT_EQ(countedDestroyed, count * perActor);
		EXPECT_EQ(registry().region(space, &actorClass), nullptr);
		EXPECT_FALSE(registry().hasSpace(space));
		EXPECT_EQ(registry().resolve(handle), nullptr);

		// Destructors ran exactly once.
		registry().housekeeping().commit();
		EXPECT_EQ(countedDestroyed, count * perActor);
	}

	TEST_F(Spawn, SpansPerType) {
		constexpr size_t count = 100uLL;
		const auto stride = Body::actorClass().desc().stride;
		static_cast<void>(SpawnActors(&Body::actorClass(), count, noInit, _universe).get());

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
