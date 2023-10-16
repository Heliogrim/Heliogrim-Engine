#pragma once
#include <Engine.Common/Wrapper.hpp>
#include <Engine.GFX.Acc/AccelerationEffect.hpp>

#include "Profile/EffectProfile.hpp"
#include "Spec/EffectSpecification.hpp"

namespace hg::engine::gfx::acc {
    struct EffectCompileRequest final {
        smr<AccelerationEffect> effect;
        smr<const EffectProfile> profile;
        cref<EffectSpecification> spec;
    };
}
