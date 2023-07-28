#pragma once

#include <Engine.Common/Collection/DenseSet.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>

#include "SignalStage.hpp"

namespace hg::engine::gfx::render::pipeline {
    class DivergingStage :
        public SignalStage {
    public:
        DivergingStage() noexcept = default;

        ~DivergingStage() noexcept override = default;

    public:
        void operator()(nmpt<State> state_) const override;
    };
}
