#pragma once
#include "PassBuilder.hpp"

namespace hg::engine::gfx::acc {
    class VkPassBuilder final :
        public PassBuilder {
    public:
        VkPassBuilder();

        ~VkPassBuilder() override;

    public:
        [[nodiscard]] smr<AccelerationPass> build(
            mref<smr<AccelerationEffect>> effect_,
            cref<class SpecificationStorage> specifications_,
            cref<smr<const class EffectProfile>> profile_
        ) const noexcept override;
    };
}
