#pragma once

#include "Stage.hpp"

#include "../Invocation/InvocationGenerator.hpp"

namespace hg::engine::gfx::render::pipeline {
    class __declspec(novtable) SubpassStage :
        public Stage {
    protected:
        SubpassStage();

    public:
        ~SubpassStage() noexcept override = default;

    public:
        void operator()(nmpt<State> state_) const override = 0;

    protected:
        uptr<InvocationGenerator> _ig;
    };
}
