#pragma once

#include <Engine.Accel.Effect/AccelerationEffect.hpp>

#include "__fwd.hpp"

namespace hg::engine::gfx::material {
    struct MaterialEffect {
        smr<const accel::AccelerationEffect> effect;
        ptr<EffectUsagePattern> pattern;
    };
}
