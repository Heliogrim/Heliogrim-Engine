#pragma once
#include <Engine.Accel.Effect/AccelerationEffect.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>

#include "Profile/EffectProfile.hpp"
#include "Spec/EffectSpecification.hpp"

namespace hg::engine::accel {
    struct EffectCompileRequest final {
        smr<const AccelerationEffect> effect;
        smr<const EffectProfile> profile;
        smr<const EffectSpecification> spec;
    };
}
