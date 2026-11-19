#pragma once
#include <Engine.Accel.Effect/AccelerationEffect.hpp>
#include <Engine.Common/__macro.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::render {
    [[nodiscard]] extern smr<const accel::AccelerationEffect> makeBrdfPrefilterEffect();
}
