#pragma once

#include "Stage.hpp"

namespace hg::engine::gfx::render::pipeline {
    class InterlockStage :
        public Stage {
    public:
        InterlockStage() noexcept = default;

        ~InterlockStage() noexcept override = default;

    public:
        void reset() override;

        void operator()(nmpt<State> state_) const override;
    };
}
