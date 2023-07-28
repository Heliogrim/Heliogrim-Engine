#pragma once

#include <Engine.Common/Collection/DenseSet.hpp>
#include <Engine.Common/Collection/Vector.hpp>

#include "SignalStage.hpp"

namespace hg::engine::gfx::render::pipeline {
    class ConvergingStage :
        public SignalStage {
    public:
        ConvergingStage() noexcept = default;

        ~ConvergingStage() noexcept override = default;

    public:
        [[nodiscard]] bool ready() const noexcept override;

        void operator()(nmpt<State> state_) const override;

    private:
        Vector<decltype(ExecState::cnd)> _predSignals;

    public:
        void addPredSignal(decltype(ExecState::cnd) signal_);

        [[nodiscard]] cref<Vector<decltype(ExecState::cnd)>> getPredSignals();

        void removePredSignal(decltype(ExecState::cnd) signal_);
    };
}
