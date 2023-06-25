#pragma once
#include "PassBuilder.hpp"

namespace hg::engine::gfx::acc {
    class DxPassBuilder final :
        public PassBuilder {
    public:
        DxPassBuilder();

        ~DxPassBuilder() override;

    public:
        [[nodiscard]] smr<AccelerationPass> build(
            mref<smr<AccelerationEffect>> effect_,
            cref<class SpecificationStorage> specifications_
        ) const noexcept override;
    };
}
