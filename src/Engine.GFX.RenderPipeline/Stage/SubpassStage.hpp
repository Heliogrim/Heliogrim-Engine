#pragma once

#include "Stage.hpp"

namespace hg::engine::gfx::render::pipeline {
    class __declspec(novtable) SubpassStage :
        public Stage {
    protected:
        SubpassStage() noexcept;

    public:
        ~SubpassStage() noexcept override = default;

    public:
        void operator()(nmpt<State> state_) const override = 0;
    };
}
