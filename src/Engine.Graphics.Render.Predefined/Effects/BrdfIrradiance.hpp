#pragma once
#include <Engine.Accel.Effect/AccelerationEffect.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>

namespace hg::engine::render {
    [[nodiscard]] extern smr<const accel::AccelerationEffect> makeBrdfIrradianceEffect();
}
