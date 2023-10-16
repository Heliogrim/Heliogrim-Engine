#pragma once

#include <Engine.GFX.Acc/AccelerationPass.hpp>
#include <Engine.GFX.Acc.Compile/Spec/EffectSpecification.hpp>

#include "SubpassStage.hpp"

namespace hg::engine::gfx::render::pipeline {
    class AccelSubpassStage :
        public SubpassStage {
    public:
        using this_type = AccelSubpassStage;

        using StagedAccelPass = _STD pair<smr<const acc::EffectSpecification>, smr<const acc::AccelerationPass>>;

    public:
        AccelSubpassStage() noexcept;

        AccelSubpassStage(mref<StagedAccelPass> stagedPass_) noexcept;

        ~AccelSubpassStage() override;

    public:
        void operator()(nmpt<State> state_) const override;

    private:
        StagedAccelPass _stagedAccelPass;
    };
}
