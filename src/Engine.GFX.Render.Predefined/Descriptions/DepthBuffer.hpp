#pragma once

// TODO: Create a symbol table, which is provided by the context where to use
// TODO: We might have "globalized" symbols which may vary in there specific description
// TODO:    e.g. depth buffer -> Depth | Stencil | Depth Stencil + F16 | F24 | F32

#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.GFX.RenderGraph/Relation/Description.hpp>

namespace hg::engine::render {
    [[nodiscard]] extern smr<const graph::Description> makeDepthBufferDescription();
}
