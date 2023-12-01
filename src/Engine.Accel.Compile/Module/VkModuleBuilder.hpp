#pragma once

#include "ModuleBuilder.hpp"

namespace hg::engine::accel {
    class VkModuleBuilder final :
        public ModuleBuilder {
    public:
        using this_type = VkModuleBuilder;

        VkModuleBuilder() = default;

        ~VkModuleBuilder() noexcept override;

    private:
        [[nodiscard]] bool isFirstStage(
            cref<smr<AccelerationPipeline>> targetPass_,
            cref<smr<StageDerivat>> stage_
        ) const noexcept;

    private:
        [[nodiscard]] uptr<struct VkModuleSource> transpile(
            cref<smr<StageDerivat>> stage_,
            mref<uptr<struct VkModuleSource>> module_
        ) const;

    public:
        [[nodiscard]] uptr<struct ModuleSource> build(
            cref<smr<AccelerationPipeline>> targetPass_,
            cref<smr<const class EffectSpecification>> specifications_,
            cref<smr<StageDerivat>> stage_,
            cref<uptr<struct ModuleSource>> previous_
        ) const override;
    };
}
