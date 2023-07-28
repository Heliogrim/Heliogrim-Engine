#pragma once

#include <Engine.Common/Wrapper.hpp>

#include "Stage.hpp"

namespace hg::engine::gfx::render::pipeline {
    class __declspec(novtable) SignalStage :
        public Stage {
    public:
        using this_type = SignalStage;

    protected:
        SignalStage() noexcept = default;

    public:
        ~SignalStage() noexcept override = default;
    };
}
