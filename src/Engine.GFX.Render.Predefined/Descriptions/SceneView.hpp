#pragma once
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.GFX.RenderGraph/Relation/Description.hpp>

namespace hg::engine::gfx::render {
    [[nodiscard]] extern smr<const graph::Description> makeSceneViewDescription();
}
