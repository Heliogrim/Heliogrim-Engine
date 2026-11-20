#pragma once

#include <Engine.Graphics.Scene/View/SceneView.hpp>

#include "../Memory/GraphNodeAllocator.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

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

		[[nodiscard]] nmpt<const graphics::scene::SceneView> getSceneView() const noexcept;
	};
}
