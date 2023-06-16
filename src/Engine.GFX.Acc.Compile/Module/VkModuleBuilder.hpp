#pragma once
#include "ModuleBuilder.hpp"

namespace hg::engine::gfx::acc {
    class VkModuleBuilder final :
        public ModuleBuilder {
    public:
        using this_type = VkModuleBuilder;

        VkModuleBuilder() = default;

        ~VkModuleBuilder() noexcept override;

    public:
        [[nodiscard]] ModuleSource build(
            cref<smr<AccelerationPass>> targetPass_,
            cref<ScopedTokenStorage> scopedTokens_,
            cref<smr<AccelerationStageDerivat>> stage_
        ) const override;
    };
}
