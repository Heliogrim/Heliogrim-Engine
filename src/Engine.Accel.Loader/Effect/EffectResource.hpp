#pragma once
#include <Engine.Accel.Effect/AccelerationEffect.hpp>
#include <Engine.Resource/Manage/Resource.hpp>

namespace hg::engine::accel {
    using AccelEffect = ::hg::engine::accel::AccelerationEffect;

    struct AccelEffectWrapper {
        smr<const AccelEffect> instance;
    };

    typedef resource::Resource<AccelEffectWrapper> EffectResource;
}
