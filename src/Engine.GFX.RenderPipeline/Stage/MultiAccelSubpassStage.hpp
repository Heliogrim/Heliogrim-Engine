#pragma once

#include <Engine.GFX.Acc/AccelerationPass.hpp>
#include <Engine.GFX.Acc.Compile/Spec/SpecificationStorage.hpp>

#include "SubpassStage.hpp"

namespace hg::engine::gfx::render::pipeline {
    class MultiAccelSubpassStage :
        public SubpassStage {
    public:
        MultiAccelSubpassStage() = default;

    public:
        void operator()(nmpt<State> state_) const override;

    private:
        Vector<_STD pair<smr<acc::SpecificationStorage>, smr<const acc::AccelerationPass>>> _accel;
    };
}
