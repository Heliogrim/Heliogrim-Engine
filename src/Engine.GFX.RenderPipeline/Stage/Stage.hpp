#pragma once

#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

#include "../__fwd.hpp"
#include "../State/ExecState.hpp"

namespace hg::engine::gfx::render::pipeline {
    class __declspec(novtable) Stage {
    public:
        using this_type = Stage;

    public:
        friend class ::hg::engine::gfx::render::RenderPipeline;

    protected:
        Stage() noexcept = default;

    public:
        virtual ~Stage() noexcept = default;

    protected:
        /**
         * Execution state handled at pipeline execution
         */
        ExecState _execState;

    public:
        void setExecStateCnd(mref<decltype(ExecState::cnd)> execStateCnd_);

        [[nodiscard]] cref<ExecState> getExecState() const noexcept;

    public:
        /**
         * State handling for this stage, prior to the execution
         */

        /**
         *
         */
        [[nodiscard]] _Success_(return != nullptr) const nmpt<const StateAware> getStateAware() const noexcept;

        [[nodiscard]] _Success_(return != nullptr) virtual const nmpt<StateAware> getStateAware() noexcept;

    public:
        /**
         * Execution handling
         */

        /**
         * Check whether the current stage is ready to be started
         */
        [[nodiscard]] virtual bool ready() const noexcept;

        /**
         * This is primarily used to reset the stored execution state.
         */
        virtual void reset();

        /**
         * Overloaded Execution function for the pipeline stage
         *
         * @param state_ The state to use for the execution
         *
         * @details The provided state_ will contain the pre-allocated sub-state based on the requested
         *  resources using the state aware interface.
         */
        virtual void operator()(nmpt<State> state_) const = 0;

    private:
        [[nodiscard]] bool start();

        void finish();
    };
}
