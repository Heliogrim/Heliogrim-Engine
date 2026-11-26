#include "pch.h"

/**/


import Engine.Common;
import Engine.Core;
import Engine.Event;
import Engine.Level;
import Engine.Scene;

using namespace hg;
using namespace hg::engine;
using namespace hg::engine::core;

namespace Core {
	namespace {
		class StubRegisterContext final :
			public IComponentRegisterContext {
		public:
			void add(ptr<HierarchyComponent>) override {}

			void add(ref<const ComponentHierarchy>) override {}

			void remove(ptr<HierarchyComponent>) override {}

			void remove(ref<const ComponentHierarchy>) override {}
		};

		class StubScene final :
			public scene::SceneBase {
		public:
			void prepare() override {}

			void cleanup() override {}

			void broadcast(scene::SceneBroadcastFlags) override {}

			void update(scene::SceneUpdateFlags) override {}

			void postprocess(scene::ScenePostProcessFlags) override {}

			bool add(const ptr<SceneComponent>) override { return false; }

			bool remove(const ptr<const SceneComponent>) override { return false; }

			nmpt<IComponentRegisterContext> registerContext() noexcept override { return &_ctx; }

			bool hasSceneSystem(const ptr<const MetaClass>) const noexcept override { return false; }

			nmpt<scene::SceneSystemBase> getSceneSystem(size_t) const noexcept override { return nullptr; }

			nmpt<scene::SceneSystemBase> getSceneSystem(const ptr<const MetaClass>) const noexcept override {
				return nullptr;
			}

		private:
			StubRegisterContext _ctx;
		};

		/* Lifecycle stays at eUndefined so ~Engine::tidy() has nothing to run. */
		class TestEngine final :
			public Engine {
		public:
			using Engine::enqueueRemovedUniverse;

			bool preInit() override { return true; }

			bool init() override { return true; }

			bool postInit() override { return true; }

			bool start() override { return true; }

			bool stop() override { return true; }

			bool shutdown() override { return true; }

			bool exit() override { return true; }

			ref<Config> getConfig() const noexcept override { std::unreachable(); }

			ref<GlobalEventEmitter> getEmitter() const noexcept override { return _emitter; }

			ref<core::Modules> getModules() const noexcept override { std::unreachable(); }

			ref<core::Timing> getTiming() const noexcept override { std::unreachable(); }

			std::span<const nmpt<core::UniverseContext>> getUniverseContexts() const noexcept override { return {}; }

			void addUniverse(mref<SharedPtr<core::Universe>>) override {}

			void removeUniverse(mref<SharedPtr<core::Universe>>) override {}

		private:
			mutable GlobalEventEmitter _emitter;
		};

		[[nodiscard]] SharedPtr<core::Universe> makeUniverse() {
			DenseSet<Arci<core::Level>> levels {};
			levels.insert(make_root_like_level());
			return std::make_shared<core::Universe>(make_uptr<StubScene>(), std::move(levels));
		}
	}

	TEST(UniverseDrain, EmitsRemoveEventInEnqueueOrder) {
		TestEngine engine {};

		auto a = makeUniverse();
		auto b = makeUniverse();
		auto c = makeUniverse();
		const std::vector<const core::Universe*> expected { a.get(), b.get(), c.get() };

		std::vector<const core::Universe*> seen {};
		engine.getEmitter().on<UniverseRemoveEvent>(
			[&seen](cref<UniverseRemoveEvent> event_) { seen.push_back(event_.getUniverse().get()); }
		);

		engine.enqueueRemovedUniverse(std::move(a));
		engine.enqueueRemovedUniverse(std::move(b));
		engine.enqueueRemovedUniverse(std::move(c));
		EXPECT_TRUE(engine.hasPendingRemovedUniverses());

		engine.drainRemovedUniverses();

		EXPECT_FALSE(engine.hasPendingRemovedUniverses());
		EXPECT_EQ(seen, expected);
	}

	TEST(UniverseDrain, ThrowingListenerStillReleasesBatch) {
		TestEngine engine {};

		auto a = makeUniverse();
		auto b = makeUniverse();
		std::weak_ptr<core::Universe> weakA { a };
		std::weak_ptr<core::Universe> weakB { b };

		engine.getEmitter().on<UniverseRemoveEvent>(
			[](cref<UniverseRemoveEvent>) { throw std::runtime_error { "listener" }; }
		);

		engine.enqueueRemovedUniverse(std::move(a));
		engine.enqueueRemovedUniverse(std::move(b));

		EXPECT_THROW(engine.drainRemovedUniverses(), std::runtime_error);

		EXPECT_FALSE(engine.hasPendingRemovedUniverses());
		EXPECT_TRUE(weakA.expired());
		EXPECT_TRUE(weakB.expired());
	}

	TEST(UniverseDrain, TeardownIsIdempotent) {
		const auto universe = makeUniverse();
		ASSERT_NE(universe->getRootLevel().get(), nullptr);

		universe->teardown();
		EXPECT_NO_THROW(universe->teardown());

		EXPECT_EQ(universe->getRootLevel().get(), nullptr);
		EXPECT_TRUE(universe->getLevels().empty());
	}

	TEST(UniverseDrain, EmptyDrainIsNoOp) {
		TestEngine engine {};
		bool fired = false;
		engine.getEmitter().on<UniverseRemoveEvent>([&fired](cref<UniverseRemoveEvent>) { fired = true; });

		EXPECT_FALSE(engine.hasPendingRemovedUniverses());
		engine.drainRemovedUniverses();

		EXPECT_FALSE(engine.hasPendingRemovedUniverses());
		EXPECT_FALSE(fired);
	}
}
