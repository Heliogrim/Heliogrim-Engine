#pragma once
#include <Engine.Common/Collection/DenseMap.hpp>

#include "ModuleBuilder.hpp"

namespace hg::engine::gfx::acc {
    class VkModuleBuilder final :
        public ModuleBuilder {
    public:
        using this_type = VkModuleBuilder;

        VkModuleBuilder() = default;

        ~VkModuleBuilder() noexcept override;

    public:
        [[nodiscard]] uptr<ModuleSource> build(
            cref<smr<AccelerationPass>> targetPass_,
            cref<class SpecificationStorage> specifications_,
            cref<smr<AccelerationStageDerivat>> stage_
        ) const override;
    };
}
