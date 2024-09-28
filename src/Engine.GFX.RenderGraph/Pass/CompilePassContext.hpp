#pragma once

#include <memory_resource>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.GFX.Scene/View/SceneView.hpp>

#include "../Memory/GraphNodeAllocator.hpp"

namespace hg::engine::render::graph {
	class CompilePassContext {
	public:
		using this_type = CompilePassContext;

	public:
		CompilePassContext() noexcept;

		constexpr ~CompilePassContext() noexcept = default;

	private:
		GraphNodeAllocator _allocator;

	public:
		[[nodiscard]] nmpt<const GraphNodeAllocator> getGraphNodeAllocator() const noexcept;

		[[nodiscard]] nmpt<const gfx::scene::SceneView> getSceneView() const noexcept;
	};
}
