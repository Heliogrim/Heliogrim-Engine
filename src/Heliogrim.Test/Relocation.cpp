#include "pch.h"

#include <cstddef>
#include <vector>
#include <Engine.ACS/Registry.hpp>
#include <Engine.ACS/Handle/Epoch.hpp>
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
		[[nodiscard]] const std::byte* bytes(const void* const p_) noexcept {
			return static_cast<const std::byte*>(p_);
		}

		/* Holds raw component pointers the way render and physics models do, and rebinds them through the table. */
		class MockSceneSystem final :
			public engine::scene::SceneSystemBase {
		public:
			std::vector<ptr<HierarchyComponent>> owners {};

			void prepare() override {}

			void cleanup() override {}

			void broadcast(engine::scene::SceneBroadcastFlags) override {}

			void update(engine::scene::SceneUpdateFlags) override {}

			void postprocess(engine::scene::ScenePostProcessFlags) override {}

			void add(const ptr<const MetaClass>, std::span<const ptr<SceneComponent>>) override {}

			void remove(const ptr<const MetaClass>, std::span<const ptr<const SceneComponent>>) override {}

			void clear() override {}

			void relocate(ref<const engine::acs::RelocationTable> table_) override {
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
			std::vector<ptr<Actor>> _actors;

			[[nodiscard]] ref<engine::acs::Registry> registry() const noexcept {
				return *_engine->getActors()->getRegistry();
			}

			// Empties chunk 0, the only chunk, so every actor lands in a fresh chunk.
			[[nodiscard]] engine::acs::RelocationTable moveAll() {
				registry().relocation().enqueue(registry().debugMoveRequest(_space, &Body::actorClass(), 0u));
				auto tables = registry().relocation().apply();
				EXPECT_EQ(tables.size(), 1uLL);
				engine::acs::Epoch::bump();
				return std::move(tables.front());
			}
		};
	}

	TEST_F(Relocation, ActorRefRefreshes) {
		auto* const actor = _actors[1];
		ASSERT_TRUE(actor->getRootComponent().has_value());
		auto* const root = &actor->getRootComponent().value();

		const ActorRef<Body> actorRef { registry().handleOf(*actor) };
		const ComponentRef<HierarchyComponent> componentRef {
			actorRef.handle,
			bytes(root) - bytes(actor)
		};
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
		const ActorRef<Body> detachedRef { registry().handleOf(*detached.front()) };
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
		std::vector<const std::byte*> before {};
		for (auto* const actor : _actors) {
			for (auto* const component : actor->getComponents()) {
				system.owners.push_back(component);
				before.push_back(bytes(component));
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
}
