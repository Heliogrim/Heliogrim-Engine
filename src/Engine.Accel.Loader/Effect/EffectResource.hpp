#pragma once
#include <Engine.GFX.Acc/AccelerationEffect.hpp>
#include <Engine.Resource/Manage/Resource.hpp>

namespace hg::engine::accel {
    using AccelEffect = ::hg::engine::gfx::acc::AccelerationEffect;

    struct AccelEffectWrapper {
        smr<const AccelEffect> instance;
    };

    typedef resource::Resource<AccelEffectWrapper> EffectResource;
}
