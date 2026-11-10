#pragma once

#include <Engine.Graphics.Render/Renderer.hpp>

namespace hg::editor::render {
	class EditorRenderer :
		public ::hg::engine::render::Renderer {
	public:
		using this_type = EditorRenderer;

	public:
		EditorRenderer(
			mref<nmpt<engine::graphics::cache::GlobalCacheCtrl>> globalCache_,
			mref<nmpt<engine::graphics::memory::GlobalPooledAllocator>> globalGfxAllocator_
		) noexcept;

		~EditorRenderer() noexcept override;

	private:
		[[nodiscard]] static uptr<engine::render::graph::CompileGraph> makeCompileGraph() noexcept;
	};
}
