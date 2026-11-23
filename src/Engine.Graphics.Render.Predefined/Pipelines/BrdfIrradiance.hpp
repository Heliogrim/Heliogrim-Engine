#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Accel.Pipeline;
import Engine.Common;
#endif

namespace hg::engine::render {
    [[nodiscard]] extern smr<const accel::GraphicsPipeline> makeBrdfIrradiancePipeline();
}
