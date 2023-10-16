#pragma once

#include <Engine.Common/Collection/DenseMap.hpp>
#include <Engine.Common/Collection/DenseSet.hpp>
#include <Engine.GFX.Acc/AccelerationPass.hpp>
#include <Engine.GFX.Loader/Material/MaterialResource.hpp>

#include "SubpassStage.hpp"

namespace hg::engine::gfx::render::pipeline {
    class MaterialSubpassStage :
        public SubpassStage {
    public:
        using this_type = MaterialSubpassStage;

        // Note: Do we need multiple specifications when using a material based subpass?
        // While a MultiSubpassStage is intended to reduce the amount and layout of multiple single passes
        //  or directly dependent accel passes, the material subpass serves as a composed stage to unify the
        //  invocation interface for a certain compiled description ~> should serve
        //  the same effect profile and specifications...
        // using StagedAccelPass = _STD pair<smr<acc::EffectSpecification>, smr<const acc::AccelerationPass>>;

        // Note: Do we even need the specifications used while compiling the acceleration module, cause we
        //  can already query against he module itself, and any exposed symbol should be (value) invariant
        //  to a derivat of the same module exposing the same symbol.
        //  Additionally if any material translates into a multi-pass sub-stage, we would need to capture
        //  all the sub-specifications used to generate every single acceleration module.

        using StagedAccelPass = smr<const acc::AccelerationPass>;

    public:
        MaterialSubpassStage() noexcept;

        MaterialSubpassStage(
            mref<DenseMap<smr<const MaterialResource>, Vector<StagedAccelPass>>> stagedPasses_
        ) noexcept;

        ~MaterialSubpassStage() override;

    public:
        void operator()(nmpt<State> state_) const override;

    private:
        DenseMap<smr<const MaterialResource>, Vector<StagedAccelPass>> _stagedAccelPasses;

    public:
        /*
        [[nodiscard]] Vector<StagedAccelPass> getAccelByMaterial(
            mref<smr<const MaterialResource>> material_
        ) noexcept;
        */
    };
}
