#include "pch.h"

/**/

#include <atomic>
#include <thread>
#include <vector>
#include <Engine.Common/Make.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Scene/SceneBase.hpp>
#include <Engine.Scene.Game/SceneComponentQueue.hpp>
#include <Heliogrim/Component/CachedActorPointer.hpp>
#include <Heliogrim/Component/Scene/Collision/BoxShapeCollisionComponent.hpp>

using namespace hg::engine::scene;
using namespace hg;

namespace HeliogrimApi::scene_queue {
	class CountingScene final :
		public SceneBase,
		public SceneComponentQueue<CountingScene> {
	public:
		size_t added = 0uLL;
		size_t removed = 0uLL;

		void commit() {
			commitChanges<>();
		}

		void add(const ptr<const MetaClass>, std::span<const ptr<SceneComponent>> components_) {
			added += components_.size();
		}

		void remove(const ptr<const MetaClass>, std::span<const ptr<const SceneComponent>> components_) {
			removed += components_.size();
		}

		void prepare() override {}

		void cleanup() override {}

		void broadcast(SceneBroadcastFlags) override {}

		void update(SceneUpdateFlags) override {}

		void postprocess(ScenePostProcessFlags) override {}

		bool add(const ptr<SceneComponent>) override {
			return false;
		}

		bool remove(const ptr<const SceneComponent>) override {
			return false;
		}

		[[nodiscard]] nmpt<IComponentRegisterContext> registerContext() noexcept override {
			return this;
		}

		[[nodiscard]] bool hasSceneSystem(const ptr<const MetaClass>) const noexcept override {
			return false;
		}

		[[nodiscard]] nmpt<SceneSystemBase> getSceneSystem(size_t) const noexcept override {
			return nullptr;
		}

		[[nodiscard]] nmpt<SceneSystemBase> getSceneSystem(const ptr<const MetaClass>) const noexcept override {
			return nullptr;
		}
	};

	TEST(SceneComponentQueue, ConcurrentQueueingWhileCommittingLosesNothing) {
		constexpr size_t producers = 4uLL;
		constexpr size_t perProducer = 2000uLL;

		Vector<uptr<BoxShapeCollisionComponent>> components {};
		for (size_t idx = 0; idx < producers * perProducer; ++idx) {
			components.emplace_back(make_uptr<BoxShapeCollisionComponent>(ComponentGuid {}, CachedActorPointer {}, nullptr));
		}

		CountingScene scene {};
		std::atomic_bool done { false };

		std::thread committer {
			[&scene, &done]() {
				while (not done.load(std::memory_order_acquire)) {
					scene.commit();
				}
			}
		};

		std::vector<std::thread> threads {};
		for (size_t producer = 0; producer < producers; ++producer) {
			threads.emplace_back(
				[&scene, &components, producer]() {
					const auto context = scene.registerContext();
					for (size_t idx = 0; idx < perProducer; ++idx) {
						auto* const component = components[producer * perProducer + idx].get();
						context->add(component);
						context->remove(component);
					}
				}
			);
		}

		for (auto& thread : threads) {
			thread.join();
		}
		done.store(true, std::memory_order_release);
		committer.join();
		scene.commit();

		EXPECT_EQ(scene.added, producers * perProducer);
		EXPECT_EQ(scene.removed, producers * perProducer);
	}
}
