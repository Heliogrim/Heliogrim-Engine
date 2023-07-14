#pragma once

#include <Engine.Common/Collection/DenseSet.hpp>
#include <Engine.GFX.Acc.Compile/Spec/SpecificationStorage.hpp>
#include <Engine.GFX.Loader/Material/MaterialResource.hpp>

#include "SubpassStage.hpp"

namespace hg::engine::gfx::render::pipeline {
    class MaterialSubpassStage :
        public SubpassStage {
    public:
        MaterialSubpassStage() = default;

    public:
        void operator()(nmpt<State> state_) const override;

    private:
        DenseSet<smr<acc::SpecificationStorage>> _specs;
        DenseSet<smr<MaterialResource>> _materials;

    public:
        [[nodiscard]] smr<const acc::AccelerationPass> getAccelByMaterial(
            mref<smr<MaterialResource>> material_,
            mref<smr<acc::SpecificationStorage>> spec_
        ) noexcept;
    };
}
