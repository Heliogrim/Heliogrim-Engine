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
            cref<smr<StageDerivat>> stage_
        ) const noexcept;

    private:
        [[nodiscard]] string transpile(cref<TransferToken> token_) const;

        [[nodiscard]] uptr<struct VkModuleSource> transpile(
            cref<smr<StageDerivat>> stage_,
            mref<uptr<struct VkModuleSource>> module_
        ) const;

    public:
        [[nodiscard]] uptr<struct ModuleSource> build(
            cref<smr<AccelerationPass>> targetPass_,
            cref<class EffectSpecification> specifications_,
            cref<smr<StageDerivat>> stage_,
            cref<uptr<struct ModuleSource>> previous_
        ) const override;
    };
}
