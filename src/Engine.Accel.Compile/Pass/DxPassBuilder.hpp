#pragma once
#include "PassBuilder.hpp"

namespace hg::engine::accel {
    class DxPassBuilder final :
        public PassBuilder {
    public:
        DxPassBuilder();

        ~DxPassBuilder() override;

    public:
        [[nodiscard]] smr<AccelerationPipeline> build(
            mref<smr<const AccelerationEffect>> effect_,
            cref<smr<const class EffectSpecification>> specifications_,
            cref<smr<const class EffectProfile>> profile_
        ) const noexcept override;
    };
}
