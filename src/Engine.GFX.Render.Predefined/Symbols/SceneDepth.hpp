#pragma once
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.GFX.RenderGraph/Symbol/Symbol.hpp>

namespace hg::engine::render {
    [[nodiscard]] extern smr<const graph::Symbol> makeSceneDepthSymbol();
}
