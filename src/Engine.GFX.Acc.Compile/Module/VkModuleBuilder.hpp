#pragma once

#include "ModuleBuilder.hpp"

namespace hg::engine::gfx::acc {
    class VkModuleBuilder final :
        public ModuleBuilder {
    public:
        using this_type = VkModuleBuilder;

        VkModuleBuilder() = default;

        ~VkModuleBuilder() noexcept override;

    private:
        [[nodiscard]] bool isFirstStage(
            cref<smr<AccelerationPass>> targetPass_,
            cref<smr<AccelerationStageDerivat>> stage_
        ) const noexcept;

    private:
        [[nodiscard]] string transpile(cref<AccelerationStageTransferToken> token_) const;

        [[nodiscard]] uptr<class VkModuleSource> transpile(
            cref<smr<AccelerationStageDerivat>> stage_,
            mref<uptr<class VkModuleSource>> module_
        ) const;

    public:
        [[nodiscard]] uptr<class ModuleSource> build(
            cref<smr<AccelerationPass>> targetPass_,
            cref<class SpecificationStorage> specifications_,
            cref<smr<AccelerationStageDerivat>> stage_,
            cref<uptr<class ModuleSource>> previous_
        ) const override;
    };
}
