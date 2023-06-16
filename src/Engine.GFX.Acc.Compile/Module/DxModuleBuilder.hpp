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
        [[nodiscard]] ModuleSource build(
            cref<smr<AccelerationPass>> targetPass_,
            cref<ScopedTokenStorage> scopedTokens_,
            cref<smr<AccelerationStageDerivat>> stage_
        ) const override;
    };
}
