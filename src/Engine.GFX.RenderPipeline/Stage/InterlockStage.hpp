#pragma once

#include "Stage.hpp"

namespace hg::engine::gfx::render::pipeline {
    class InterlockStage :
        public Stage {
    public:
        InterlockStage() noexcept = default;

        ~InterlockStage() noexcept override = default;
    };
}
