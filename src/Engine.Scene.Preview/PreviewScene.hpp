#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Render.Scene/RenderSceneSystem.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Scene;
#endif

namespace hg::engine::scene {
	class PreviewScene :
		public Scene<render::RenderSceneSystem>,
		public IComponentRegisterContext {
	public:
		using this_type = PreviewScene;

	public:
		void add(_In_ ptr<HierarchyComponent> component_) override;

		void add(cref<ComponentHierarchy> hierarchy_) override;

		void remove(ptr<HierarchyComponent> component_) override;

		void remove(ref<const ComponentHierarchy> hierarchy_) override;

	public:
		[[nodiscard]] nmpt<IComponentRegisterContext> registerContext() noexcept override;
	};
}
