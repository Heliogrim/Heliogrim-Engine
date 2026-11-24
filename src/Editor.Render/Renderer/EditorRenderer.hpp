#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Graphics.Render;
#endif

namespace hg::editor::render {
	class EditorRenderer :
		public ::hg::engine::render::Renderer {
	public:
		using this_type = EditorRenderer;

	public:
		EditorRenderer(
			mref<nmpt<engine::graphics::cache::GlobalCacheCtrl>> globalCache_,
			mref<nmpt<engine::graphics::memory::GlobalPooledAllocator>> globalGraphicsAllocator_
		) noexcept;

		~EditorRenderer() noexcept override;

	private:
		[[nodiscard]] static uptr<engine::render::graph::CompileGraph> makeCompileGraph() noexcept;
	};
}
