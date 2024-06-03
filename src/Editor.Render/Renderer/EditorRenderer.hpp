#pragma once

#include <Engine.GFX.Render/Renderer.hpp>

namespace hg::editor::render {
	class EditorRenderer :
		public ::hg::engine::render::Renderer {
	public:
		using this_type = EditorRenderer;

	public:
		EditorRenderer(
			mref<nmpt<engine::gfx::cache::GlobalCacheCtrl>> globalCache_,
			mref<nmpt<engine::gfx::memory::GlobalPooledAllocator>> globalGfxAllocator_
		) noexcept;

		~EditorRenderer() noexcept override;

	private:
		[[nodiscard]] static uptr<engine::render::graph::CompileGraph> makeCompileGraph() noexcept;
	};
}
