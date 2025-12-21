#pragma once

#include <Engine.Render.Scene/RenderSceneSystem.hpp>
#include <Engine.Scene/Scene.hpp>
#include <Heliogrim/IComponentRegisterContext.hpp>

#include "SceneComponentQueue.hpp"

namespace hg::engine::scene {
	class GameScene final :
		public Scene<render::RenderSceneSystem>,
		public SceneComponentQueue<GameScene> {
	public:
		using this_type = GameScene;

	public:
		~GameScene() noexcept override = default;

	public:
		void add(const __restricted_ptr<const MetaClass> metaClass_, std::span<const ptr<SceneComponent>> components_) {
			forEachSystem(
				[metaClass_, components_](const ptr<SceneSystemBase> sys_) {
					sys_->add(metaClass_, components_);
				}
			);
		}

		void remove(const __restricted_ptr<const MetaClass> metaClass_, std::span<const ptr<const SceneComponent>> components_) {
			forEachSystem(
				[metaClass_, components_](const ptr<SceneSystemBase> sys_) {
					sys_->remove(metaClass_, components_);
				}
			);
		}

	public:
		void broadcast(SceneBroadcastFlags flags_) override;

	public:
		[[nodiscard]] nmpt<IComponentRegisterContext> registerContext() noexcept override;
	};
}
