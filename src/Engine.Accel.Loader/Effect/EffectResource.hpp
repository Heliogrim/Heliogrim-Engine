#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Accel.Effect;
import Engine.Resource;
#endif

namespace hg::engine::accel {
    using AccelEffect = ::hg::engine::accel::AccelerationEffect;

    struct AccelEffectWrapper {
        smr<const AccelEffect> instance;
    };

    typedef resource::Resource<AccelEffectWrapper> EffectResource;
}
