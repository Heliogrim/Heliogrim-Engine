#pragma once
#include <Engine.Accel.Pipeline/GraphicsPipeline.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>

namespace hg::engine::gfx::render {
    [[nodiscard]] extern smr<const accel::GraphicsPipeline> makeBrdfIrradiancePipeline();
}
