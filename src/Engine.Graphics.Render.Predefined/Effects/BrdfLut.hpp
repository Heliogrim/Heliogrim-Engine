#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Accel.Effect;
import Engine.Common;
#endif

namespace hg::engine::render {
    [[nodiscard]] extern smr<const accel::AccelerationEffect> makeBrdfLutEffect();
}
