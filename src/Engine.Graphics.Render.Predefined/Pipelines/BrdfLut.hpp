#pragma once
#include <Engine.Accel.Pipeline/GraphicsPipeline.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::render {
    [[nodiscard]] extern smr<const accel::GraphicsPipeline> makeBrdfLutPipeline();
}
