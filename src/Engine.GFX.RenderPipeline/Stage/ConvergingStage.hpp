#pragma once

#include <Engine.Common/Collection/DenseSet.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>

#include "SignalStage.hpp"

namespace hg::engine::gfx::render::pipeline {
    class ConvergingStage :
        public SignalStage {
    public:
        ConvergingStage() noexcept = default;

        ~ConvergingStage() noexcept override = default;

    public:
        void reset() override;

        void operator()(nmpt<State> state_) const override;

    public:
        struct Predecessor {
            // Should be ConvergingStage | DivergingStage
            nmpt<SignalStage> signalStage;
            // Can be any Stage
            nmpt<Stage> pathStage;
        };

        struct Successor {
            // Should be ConvergingStage | DivergingStage
            nmpt<SignalStage> receiverStage;
            // Can be any Stage
            nmpt<Stage> pathStage;
        };

    public:
        [[nodiscard]] cref<DenseSet<Predecessor>> getSignalPredecessors();

        [[nodiscard]] cref<Successor> getSignalSuccessor();

    private:
        // Vector<_STD atomic_flag> _signals;
        // We can just accumulate all signal flags into one atomic counter
        _STD atomic_uint_fast8_t _signalAcc;

    public:
        void signal(mref<nmpt<SignalStage>> signal_, mref<nmpt<Stage>> path_) override {

            const auto& set = getSignalPredecessors();
            const auto sigIt = set.find({ signal_, path_ });

            // Would be required, if we did not accumulate the atomic flags
            // const auto index = _STD distance(set.begin(), sigIt);

            const auto left = _signalAcc.fetch_sub(1, _STD memory_order::acq_rel);
            if (left != 0) {
                return;
            }

            // If we get the last signal, so we expect no more executions, we can implicitly finalize this.
            finalize();
        }

        void finalize() {
            const auto& successor = getSignalSuccessor();
            successor.receiverStage->signal(this, clone(successor.pathStage));
        }
    };
}
