#pragma once
#include <atomic>
#include <Engine.Common/Collection/List.hpp>

#include "./Dispatchable.hpp"
#include "PipelineStage.hpp"

namespace ember::engine::utils {
    class Pipeline {
    public:
        /**
         * Constructor
         */
        Pipeline();

        /**
         * Destructor
         */
        ~Pipeline() = default;

    private:
        /**
         * Pipeline Stages
         */
        vector<PipelineStage> _stages;

    public:
        void addStage(const PipelineStage& stage_);

        void addStage(Dispatchable& dispatchable_);

        void clearStages();

    private:
        /**
         * Running State
         */
        _STD atomic_bool _running;

    public:
        bool running() const;

    private:
        /**
         * Mutation
         */

    public:
        /**
         * Executes the given pipeline
         *
         * @author Julius
         * @date 14.05.2020
         *
         * @returns True if invocation succeeds, false if it fails.
         */
        bool invoke();
    };
}
