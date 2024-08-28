#pragma once
#include <Engine.Render.Scene/RenderSceneSystem.hpp>
#include <Engine.Scene/Scene.hpp>
#include <Heliogrim/IComponentRegisterContext.hpp>

namespace hg::engine::scene {
	class GameScene final :
		public Scene<render::RenderSceneSystem>,
		public IComponentRegisterContext {
	public:
		using this_type = GameScene;

	public:
		~GameScene() noexcept override = default;

	public:
		void add(_In_ ptr<HierarchyComponent> component_) override;

		void add(cref<ComponentHierarchy> hierarchy_) override;

	public:
		[[nodiscard]] nmpt<IComponentRegisterContext> registerContext() noexcept override;
	};
}
