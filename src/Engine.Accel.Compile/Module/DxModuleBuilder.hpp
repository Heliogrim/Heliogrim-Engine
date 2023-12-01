#pragma once
#include "ModuleBuilder.hpp"

namespace hg::engine::accel {
    class DxModuleBuilder final :
        public ModuleBuilder {
    public:
        using this_type = DxModuleBuilder;

    public:
        DxModuleBuilder() = default;

        ~DxModuleBuilder() noexcept override;

    public:
        [[nodiscard]] uptr<ModuleSource> build(
            cref<smr<AccelerationPipeline>> targetPass_,
            cref<smr<const class EffectSpecification>> specifications_,
            cref<smr<StageDerivat>> stage_,
            cref<uptr<ModuleSource>> previous_
        ) const override;
    };
}
