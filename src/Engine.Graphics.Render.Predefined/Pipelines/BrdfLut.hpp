#pragma once
#include <Engine.Accel.Pipeline/GraphicsPipeline.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>

namespace hg::engine::render {
    [[nodiscard]] extern smr<const accel::GraphicsPipeline> makeBrdfLutPipeline();
}
