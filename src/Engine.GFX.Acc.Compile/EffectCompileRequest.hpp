#pragma once
#include <Engine.Common/Wrapper.hpp>
#include <Engine.GFX.Acc/AccelerationEffect.hpp>

#include "Profile/EffectProfile.hpp"
#include "Spec/EffectSpecification.hpp"

namespace hg::engine::gfx::acc {
    struct EffectCompileRequest final {
        smr<const AccelerationEffect> effect;
        smr<const EffectProfile> profile;
        smr<const EffectSpecification> spec;
    };
}
