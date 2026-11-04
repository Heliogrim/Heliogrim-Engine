#include "pch.h"

/**/

#include <cstddef>
#include <vector>
#include <Engine.ACS/Registry.hpp>
#include <Engine.ACS/Handle/Epoch.hpp>
#include <Engine.Common/Move.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Scene/SceneSystem.hpp>
#include <Heliogrim/Actor/ActorRef.hpp>
#include <Heliogrim/Actor/Archetype/ArchetypeDefinition.inl.hpp>
#include <Heliogrim/Component/ComponentRef.hpp>

#include "SpawnEngine.hpp"
#include "TestArchetypes.hpp"

using namespace HeliogrimApi;
using namespace hg;

// gtest TEST bodies declare a member function TestBody(), which hides the fixture archetype of that name.
using Body = HeliogrimApi::TestBody;

namespace HeliogrimApi {
	namespace {
		[[nodiscard]] ptr<const std::byte> bytes(const ptr<const void> p_) noexcept {
			return static_cast<const std::byte*>(p_);
		}

		/* Holds raw component pointers the way render and physics models do, and rebinds them through the table. */
		class MockSceneSystem final :
			public engine::scene::SceneSystemBase {
		public:
			Vector<ptr<HierarchyComponent>> owners {};

			void prepare() override {}

			void cleanup() override {}

			void broadcast(engine::scene::SceneBroadcastFlags) override {}

			void update(engine::scene::SceneUpdateFlags) override {}

			void postprocess(engine::scene::ScenePostProcessFlags) override {}

			void add(const ptr<const MetaClass>, std::span<const ptr<SceneComponent>>) override {}

			void remove(const ptr<const MetaClass>, std::span<const ptr<const SceneComponent>>) override {}

			void clear() override {}

			Vector<u64> epochs {};

			void relocate(ref<const engine::acs::RelocationTable> table_) override {
				epochs.emplace_back(engine::acs::Epoch::current());
				for (auto& owner : owners) {
					if (auto* const to = table_.lookup(bytes(owner)); to != nullptr) {
						owner = reinterpret_cast<ptr<HierarchyComponent>>(to);
					}
				}
			}
		};

		class Relocation :
			public ::testing::Test {
		protected:
			void SetUp() override {
				_engine = make_uptr<test_engine::SpawnEngine>();
				_space = registry().acquireSpace();
				_actors = registry().createActors(&Body::actorClass(), 3uLL, _space);
			}

			void TearDown() override {
				_actors.clear();
				_engine.reset();
			}

			uptr<test_engine::SpawnEngine> _engine;
			engine::acs::SpaceId _space {};
			Vector<ptr<Actor>> _actors;

			[[nodiscard]] ref<engine::acs::Registry> registry() const noexcept {
				return *_engine->getActors()->getRegistry();
			}

			// Empties chunk 0, the only chunk, so every actor lands in a fresh chunk.
			[[nodiscard]] engine::acs::RelocationTable moveAll() {
				registry().relocation().enqueue(registry().debugMoveRequest(_space, &Body::actorClass(), 0u).value());
				auto tables = registry().relocation().apply();
				EXPECT_EQ(tables.size(), 1uLL);
				engine::acs::Epoch::bump();
				return ::hg::move(tables.front());
			}
		};
	}

	TEST_F(Relocation, ActorRefRefreshes) {
		auto* const actor = _actors[1];
		ASSERT_TRUE(actor->getRootComponent().has_value());
		auto* const root = &actor->getRootComponent().value();

		const auto actorRef = ActorRef<Body>::of(static_cast<ptr<const Body>>(actor)).value();
		const auto componentRef = ComponentRef<HierarchyComponent>::of(root).value();
		ASSERT_EQ(componentRef.offset(), bytes(root) - bytes(actor));
		ASSERT_TRUE(actorRef.get().has_value());
		EXPECT_EQ(static_cast<ptr<Actor>>(&actorRef.get().value()), actor);
		EXPECT_EQ(&componentRef.get().value(), root);

		const auto table = moveAll();
		auto* const moved = reinterpret_cast<ptr<Actor>>(table.lookup(bytes(actor)));
		ASSERT_NE(moved, nullptr);
		ASSERT_NE(moved, actor);

		ASSERT_TRUE(actorRef.get().has_value());
		EXPECT_EQ(static_cast<ptr<Actor>>(&actorRef.get().value()), moved);
		EXPECT_EQ(bytes(&componentRef.get().value()), table.lookup(bytes(root)));

		// Detached handles are not forwarded: they go stale on mount by design.
		const auto detached = registry().createActors(&Body::actorClass(), 1uLL, engine::acs::detachedSpace);
		const auto detachedRef = ActorRef<Body>::of(static_cast<ptr<const Body>>(detached.front())).value();
		EXPECT_FALSE(detachedRef.get().has_value());
	}

	TEST_F(Relocation, ComponentParentsValid) {
		auto* const actor = _actors[0];
		auto* const oldRoot = &actor->getRootComponent().value();
		const auto expected = std::ranges::distance(actor->getComponents());

		const auto table = moveAll();
		auto* const moved = reinterpret_cast<ptr<Actor>>(table.lookup(bytes(actor)));
		ASSERT_NE(moved, nullptr);

		// The archetype relocate op re-stamps owner and parent; every link must point into the new slot.
		size_t count = 0uLL;
		for (auto* const component : moved->getComponents()) {
			++count;
			EXPECT_EQ(component->getOwner(), moved);
			if (auto* const parent = component->getParentComponent(); parent != nullptr) {
				EXPECT_EQ(parent->getOwner(), moved);
				EXPECT_EQ(table.lookup(bytes(parent)), nullptr);
			}
		}
		EXPECT_EQ(count, static_cast<size_t>(expected));

		// A component outside the slot that still points at the old slot is remapped by relocate().
		HierarchyComponent stray {
			generate_component_guid(),
			CachedActorPointer { .guid = moved->guid(), .cached = actor },
			ptr<HierarchyComponent> { oldRoot }
		};
		stray.relocate(table);
		EXPECT_EQ(stray.getOwner(), moved);
		EXPECT_EQ(bytes(stray.getParentComponent()), table.lookup(bytes(oldRoot)));
		EXPECT_EQ(stray.getParentComponent(), &moved->getRootComponent().value());
	}

	TEST_F(Relocation, SceneModelRebinds) {
		MockSceneSystem system {};
		Vector<ptr<const std::byte>> before {};
		for (auto* const actor : _actors) {
			for (auto* const component : actor->getComponents()) {
				system.owners.emplace_back(component);
				before.emplace_back(bytes(component));
			}
		}

		const auto table = moveAll();
		engine::scene::SceneSystemBase& base = system;
		base.relocate(table);

		ASSERT_EQ(system.owners.size(), before.size());
		for (size_t index = 0uLL; index < before.size(); ++index) {
			EXPECT_EQ(bytes(system.owners[index]), table.lookup(before[index]));
			// The rebound component's owner already lives in the new slot.
			EXPECT_EQ(table.lookup(bytes(system.owners[index]->getOwner())), nullptr);
		}
	}

	TEST_F(Relocation, ReclaimInvalidatesRefs) {
		const auto actorRef = ActorRef<Body>::of(static_cast<ptr<const Body>>(_actors[2])).value();
		ASSERT_TRUE(actorRef.get().has_value());

		registry().destroyArchetypeActor(_actors[2]);
		const auto before = engine::acs::Epoch::current();
		registry().housekeep([](ref<const engine::acs::RelocationTable>) {});

		// Reclaim alone must bump, or the ref keeps serving the cached (now reusable) slot.
		EXPECT_EQ(engine::acs::Epoch::current(), before + 1uLL);
		EXPECT_FALSE(actorRef.get().has_value());

		// Nothing retired and nothing moved: no bump.
		registry().housekeep([](ref<const engine::acs::RelocationTable>) {});
		EXPECT_EQ(engine::acs::Epoch::current(), before + 1uLL);
	}

	TEST_F(Relocation, DestroyInvalidatesRefsBeforeHousekeep) {
		auto* const actor = _actors[1];
		const auto actorRef = ActorRef<Body>::of(static_cast<ptr<const Body>>(actor)).value();
		const auto componentRef = ComponentRef<HierarchyComponent>::of(&actor->getRootComponent().value()).value();
		ASSERT_TRUE(actorRef.get().has_value());
		ASSERT_TRUE(componentRef.get().has_value());

		registry().destroyArchetypeActor(actor);
		EXPECT_FALSE(actorRef.get().has_value());
		EXPECT_FALSE(componentRef.get().has_value());
	}

	TEST_F(Relocation, CorruptComponentOffsetDoesNotResolve) {
		auto* const actor = _actors[1];
		auto componentRef = ComponentRef<HierarchyComponent>::of(&actor->getRootComponent().value()).value();
		ASSERT_TRUE(componentRef.get().has_value());

		// As a corrupt layout fixup would restore it: same live handle, offset off the component grid.
		archetype_detail::RefAccess::offset(componentRef) += 1;
		archetype_detail::RefAccess::epoch(componentRef) = ~0uLL;
		EXPECT_FALSE(componentRef.get().has_value());
	}

	TEST_F(Relocation, MoveInvalidatesRefsBeforeHousekeep) {
		auto* const actor = _actors[1];
		const auto actorRef = ActorRef<Body>::of(static_cast<ptr<const Body>>(actor)).value();
		const auto componentRef = ComponentRef<HierarchyComponent>::of(&actor->getRootComponent().value()).value();
		ASSERT_TRUE(actorRef.get().has_value());
		ASSERT_TRUE(componentRef.get().has_value());

		const auto other = registry().acquireSpace();
		auto* const moved = registry().moveActor(actor, other);
		ASSERT_NE(moved, actor);
		EXPECT_FALSE(actorRef.get().has_value());
		EXPECT_FALSE(componentRef.get().has_value());
	}

	TEST_F(Relocation, MultiChunkApplySkipsStale) {
		const auto* const region = registry().region(_space, &Body::actorClass());
		ASSERT_NE(region, nullptr);
		for (size_t guard = 0uLL; region->chunkCount() < 2u && guard < 100000uLL; ++guard) {
			const auto more = registry().createActors(&Body::actorClass(), 1uLL, _space);
			_actors.insert(_actors.end(), more.begin(), more.end());
		}
		ASSERT_EQ(region->chunkCount(), 2u);

		Vector<ActorRef<Body>> refs {};
		for (auto* const actor : _actors) {
			refs.emplace_back(ActorRef<Body>::of(static_cast<ptr<const Body>>(actor)).value());
		}

		// Ascending enqueue on purpose: apply must reorder so chunk 0's move cannot shift chunk 1.
		auto& barrier = registry().relocation();
		barrier.enqueue(registry().debugMoveRequest(_space, &Body::actorClass(), 0u).value());
		barrier.enqueue(registry().debugMoveRequest(_space, &Body::actorClass(), 1u).value());
		barrier.enqueue(registry().debugMoveRequest(_space, &Body::actorClass(), 1u).value());
		barrier.enqueue(registry().debugMoveRequest(_space, &Body::actorClass(), 99u).value());
		barrier.enqueue(engine::acs::MoveRequest { nullptr, 0u, nullptr, 0u });

		// Absent class in a space creates no region.
		const auto empty = registry().acquireSpace();
		EXPECT_FALSE(registry().debugMoveRequest(empty, &Body::actorClass(), 0u).has_value());
		EXPECT_EQ(registry().region(empty, &Body::actorClass()), nullptr);

		size_t tables = 0uLL;
		registry().housekeep([&tables](ref<const engine::acs::RelocationTable>) { ++tables; });
		EXPECT_EQ(tables, 2uLL);

		for (const auto& actorRef : refs) {
			ASSERT_TRUE(actorRef.get().has_value());
			auto* const actor = static_cast<ptr<Actor>>(&actorRef.get().value());
			EXPECT_EQ(registry().handleOf(*actor), actorRef.handle());
			EXPECT_EQ(actor->getRootComponent().value().getOwner(), actor);
		}
	}

	TEST_F(Relocation, ReleaseSpaceDropsPendingMoves) {
		registry().relocation().enqueue(registry().debugMoveRequest(_space, &Body::actorClass(), 0u).value());
		registry().releaseSpace(_space);
		_actors.clear();

		size_t tables = 0uLL;
		registry().housekeep([&tables](ref<const engine::acs::RelocationTable>) { ++tables; });
		EXPECT_EQ(tables, 0uLL);
	}

	TEST_F(Relocation, MoveAppliesBeforeReclaim) {
		const auto* const region = registry().region(_space, &Body::actorClass());
		ASSERT_NE(region, nullptr);
		for (size_t guard = 0uLL; region->chunkCount() < 2u && guard < 100000uLL; ++guard) {
			const auto more = registry().createActors(&Body::actorClass(), 1uLL, _space);
			_actors.insert(_actors.end(), more.begin(), more.end());
		}
		ASSERT_EQ(region->chunkCount(), 2u);

		// Only the last actor lives in chunk 1; emptying chunk 0 makes reclaim free it and shift chunk 1 to index 0.
		const auto lastRef = ActorRef<Body>::of(static_cast<ptr<const Body>>(_actors.back())).value();
		for (size_t index = 0uLL; index + 1uLL < _actors.size(); ++index) {
			registry().destroyArchetypeActor(_actors[index]);
		}

		registry().relocation().enqueue(registry().debugMoveRequest(_space, &Body::actorClass(), 1u).value());
		size_t tables = 0uLL;
		registry().housekeep([&tables](ref<const engine::acs::RelocationTable>) { ++tables; });
		EXPECT_EQ(tables, 1uLL);

		ASSERT_TRUE(lastRef.get().has_value());
		auto* const moved = static_cast<ptr<Actor>>(&lastRef.get().value());
		EXPECT_NE(moved, _actors.back());
		EXPECT_EQ(registry().handleOf(*moved), lastRef.handle());
		_actors.clear();
	}

	TEST_F(Relocation, HousekeepOrder) {
		auto* const actor = _actors[0];
		auto* const root = &actor->getRootComponent().value();
		const auto actorRef = ActorRef<Body>::of(static_cast<ptr<const Body>>(actor)).value();
		ASSERT_TRUE(actorRef.get().has_value());

		auto* const outside = reinterpret_cast<ptr<HierarchyComponent>>(&_space);
		MockSceneSystem system {};
		system.owners = { root, nullptr, outside };

		registry().relocation().enqueue(registry().debugMoveRequest(_space, &Body::actorClass(), 0u).value());
		const auto before = engine::acs::Epoch::current();
		engine::scene::SceneSystemBase& base = system;
		registry().housekeep([&base](ref<const engine::acs::RelocationTable> table_) { base.relocate(table_); });

		// Relocate runs before the bump, exactly once per table.
		ASSERT_EQ(system.epochs.size(), 1uLL);
		EXPECT_EQ(system.epochs.front(), before);
		EXPECT_EQ(engine::acs::Epoch::current(), before + 1uLL);

		auto* const moved = static_cast<ptr<Actor>>(&actorRef.get().value());
		EXPECT_NE(moved, actor);
		EXPECT_EQ(system.owners[0], &moved->getRootComponent().value());
		EXPECT_EQ(system.owners[1], nullptr);
		EXPECT_EQ(system.owners[2], outside);
	}
}
