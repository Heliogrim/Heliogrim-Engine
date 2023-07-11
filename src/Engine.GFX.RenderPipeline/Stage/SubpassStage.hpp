#pragma once

#include "Stage.hpp"

namespace hg::engine::gfx::render::pipeline {
    class SubpassStage :
        public Stage {
    public:
        SubpassStage();

        ~SubpassStage() noexcept override = default;
    };
}
