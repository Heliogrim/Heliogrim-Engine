#pragma once
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.Graphics.RenderGraph/Relation/Description.hpp>

namespace hg::engine::render {
    [[nodiscard]] extern smr<const graph::Description> makeStencilBufferDescription();
}
