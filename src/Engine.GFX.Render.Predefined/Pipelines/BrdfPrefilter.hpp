#pragma once
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.GFX.Acc/Pipeline/GraphicsPipeline.hpp>

namespace hg::engine::gfx::render {
    [[nodiscard]] extern smr<const acc::GraphicsPipeline> makeBrdfPrefilterPipeline();
}
