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

    public:
        /**
         * Signals a unused path for the current state
         *
         * @alias signalUnused
         */
        virtual void signal(mref<nmpt<SignalStage>> signal_, mref<nmpt<Stage>> path_) = 0;
    };
}
