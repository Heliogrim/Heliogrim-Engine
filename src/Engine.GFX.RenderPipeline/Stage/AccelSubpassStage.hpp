#pragma once

#include <Engine.GFX.Acc/AccelerationPass.hpp>
#include <Engine.GFX.Acc.Compile/Spec/SpecificationStorage.hpp>

#include "SubpassStage.hpp"

namespace hg::engine::gfx::render::pipeline {
    class AccelSubpassStage :
        public SubpassStage {
    public:
        AccelSubpassStage();

        ~AccelSubpassStage() override;

    public:
        void operator()(nmpt<State> state_) const override;

    private:
        smr<acc::SpecificationStorage> _specs;
        smr<const acc::AccelerationPass> _accel;
    };
}
