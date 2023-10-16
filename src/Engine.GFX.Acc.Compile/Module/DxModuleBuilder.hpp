#pragma once
#include "ModuleBuilder.hpp"

namespace hg::engine::gfx::acc {
    class DxModuleBuilder final :
        public ModuleBuilder {
    public:
        using this_type = DxModuleBuilder;

    public:
        DxModuleBuilder() = default;

        ~DxModuleBuilder() noexcept override;

    public:
        [[nodiscard]] uptr<ModuleSource> build(
            cref<smr<AccelerationPass>> targetPass_,
            cref<class EffectSpecification> specifications_,
            cref<smr<StageDerivat>> stage_,
            cref<uptr<ModuleSource>> previous_
        ) const override;
    };
}
