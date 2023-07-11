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
        [[nodiscard]] cref<Predecessor> getSignalPredecessor();

        [[nodiscard]] cref<DenseSet<Successor>> getSignalSuccessors();

    private:
        Vector<_STD atomic_flag> _markers;

    public:
        void signal(mref<nmpt<SignalStage>> signal_, mref<nmpt<Stage>> path_) override {
            // If we get a signal, so we expect no more executions, so we can implicitly finalize this.
            finalize();
        }

        void finalize() {
            const auto& set = getSignalSuccessors();

            auto sigIt = set.begin();
            auto markIt = _markers.begin();

            while (sigIt != set.end()) {

                if (not markIt->test(_STD memory_order::consume)) {
                    sigIt->receiverStage->signal(this, clone(sigIt->pathStage));
                }

                ++sigIt;
                ++markIt;
            }
        }
    };
}
