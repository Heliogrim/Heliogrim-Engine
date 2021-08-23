#pragma once
#include "./Dispatchable.hpp"

namespace ember::engine::utils {
    /**
     * Forward declaration
     */
    class Pipeline;

    class PipelineStage {
    public:
        friend class Pipeline;

        /**
         * Default constructor
         *
         * @author Julius
         * @date 14.05.2020
         */
        PipelineStage() = delete;

        /**
         * Copy constructor
         *
         * @author Julius
         * @date 14.05.2020
         *
         * @param obj The first parameter.
         */
        PipelineStage(const PipelineStage&) = default;

        /**
         * Move constructor
         *
         * @author Julius
         * @date 14.05.2020
         *
         * @param [in,out] obj The first parameter.
         */
        PipelineStage(PipelineStage&&) = default;

        /**
         * Destructor
         *
         * @author Julius
         * @date 14.05.2020
         */
        ~PipelineStage() = default;

    protected:
        /**
         * Constructor
         *
         * @author Julius
         * @date 14.05.2020
         *
         * @param [in,out] pipeline_ The pipeline.
         * @param [in,out] dispatchable_ The dispatchable.
         */
        PipelineStage(Pipeline& pipeline_, Dispatchable& dispatchable_);

    private:
        /**
         * Pipeline
         */
        Pipeline* _pipeline;

    private:
        /**
         * Handle
         */
        Dispatchable* _dispatchable;

    private:
        /**
         * Running State
         */
        // TODO: std::atomic_bool _running;
        bool _running;

    public:
        bool running() const;

    private:
        /**
         * Mutation
         */

    public:
        void invoke();
    };
}
