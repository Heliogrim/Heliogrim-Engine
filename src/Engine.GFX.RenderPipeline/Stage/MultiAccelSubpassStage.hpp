#pragma once

#include <Engine.GFX.Acc/AccelerationPass.hpp>
#include <Engine.GFX.Acc.Compile/Spec/EffectSpecification.hpp>

#include "SubpassStage.hpp"

namespace hg::engine::gfx::render::pipeline {
    class MultiAccelSubpassStage :
        public SubpassStage {
    public:
        using this_type = MultiAccelSubpassStage;

        using StagedAccelPass = _STD pair<smr<acc::EffectSpecification>, smr<const acc::AccelerationPass>>;

    public:
        MultiAccelSubpassStage() noexcept;

        MultiAccelSubpassStage(mref<Vector<StagedAccelPass>> stagedPasses_) noexcept;

        ~MultiAccelSubpassStage() override;

    public:
        void operator()(nmpt<State> state_) const override;

    private:
        Vector<StagedAccelPass> _stagedAccelPasses;
    };
}
