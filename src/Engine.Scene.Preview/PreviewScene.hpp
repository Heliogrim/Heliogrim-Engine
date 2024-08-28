#pragma once
#include <Engine.Render.Scene/RenderSceneSystem.hpp>
#include <Engine.Scene/Scene.hpp>
#include <Heliogrim/IComponentRegisterContext.hpp>

namespace hg::engine::scene {
	class PreviewScene :
		public Scene<render::RenderSceneSystem>,
		public IComponentRegisterContext {
	public:
		using this_type = PreviewScene;

	public:
		void add(_In_ ptr<HierarchyComponent> component_) override;

		void add(cref<ComponentHierarchy> hierarchy_) override;

	public:
		[[nodiscard]] nmpt<IComponentRegisterContext> registerContext() noexcept override;
	};
}
