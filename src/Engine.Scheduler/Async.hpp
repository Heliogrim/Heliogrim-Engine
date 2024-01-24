#pragma once
#include <Engine.Common/Concurrent/Promise.hpp>
#include "Scheduler.hpp"

#include "Pipeline/Stage/StageIdentifier.hpp"

namespace hg::engine::scheduler {
    /**
     * Delays
     *
     * @author Julius
     * @date 16.11.2020
     *
     * @param [in] task_ The task.
     * @param 	   ticks_ The ticks.
     */
    void delay(_In_ mref<non_owning_rptr<const task::TaskDelegate>> task_, const hg::u32 ticks_);

    /**
     * Delays
     *
     * @author Julius
     * @date 16.11.2020
     *
     * @param [in] fnc_ The function.
     * @param 	   ticks_ The ticks.
     */
    void delay(_In_ mref<task::Task::function_type> fnc_, const hg::u32 ticks_);

    /**
     * Executes the given task
     *
     * @author Julius
     * @date 16.11.2020
     *
     * @param [in] task_ The task.
     */
    void exec(_In_ mref<non_owning_rptr<const task::TaskDelegate>> task_);

    /**
     * Executes the given function as task
     *
     * @author Julius
     * @date 16.11.2020
     *
     * @param [in] fnc_ The function.
     */
    void exec(_In_ mref<task::Task::function_type> fnc_);

    void execOnStage(
        _In_ mref<non_owning_rptr<const task::TaskDelegate>> task_,
        const non_owning_rptr<const Stage> stage_
    );

    void execOnStage(
        _In_ mref<non_owning_rptr<const task::TaskDelegate>> task_,
        cref<StageIdentifier> stage_
    );

    void execOnStage(
        _In_ mref<task::Task::function_type> fnc_,
        const non_owning_rptr<const Stage> stage_
    );

    void execOnStage(
        _In_ mref<task::Task::function_type> fnc_,
        cref<StageIdentifier> stage_
    );

    void execOnStages(
        _In_ mref<non_owning_rptr<const task::TaskDelegate>> task_,
        const non_owning_rptr<const Stage> begin_,
        const non_owning_rptr<const Stage> end_
    );

    void execOnStages(
        _In_ mref<non_owning_rptr<const task::TaskDelegate>> task_,
        cref<StageIdentifier> begin_,
        cref<StageIdentifier> end_
    );

    void execOnStages(
        _In_ mref<task::Task::function_type> task_,
        const non_owning_rptr<const Stage> begin_,
        const non_owning_rptr<const Stage> end_
    );

    void execOnStages(
        _In_ mref<task::Task::function_type> task_,
        cref<StageIdentifier> begin_,
        cref<StageIdentifier> end_
    );

    /**
     * Schedules the function and return a async resolved future
     *
     * @tparam Ty Type of the ty.
     * @param [in] fnc_ The function.
     *
     * @returns A concurrent::future&lt;Ty&gt;
     */
    template <typename Ty>
    hg::concurrent::future<Ty> async(_In_ mref<std::function<Ty()>> fnc_) {
        hg::concurrent::promise<Ty> p { fnc_ };
        hg::concurrent::future<Ty> f = p.get();

        exec({ p });

        return f;
    }

    /**
     * Schedules the function with a delay and return a async resolved future
     *
     * @tparam Ty Type of the ty.
     * @param [in] fnc_ The function.
     * @param	   ticks_ The ticks.
     *
     * @returns A concurrent::future&lt;Ty&gt;
     */
    template <typename Ty>
    hg::concurrent::future<Ty> async(_In_ mref<std::function<Ty()>> fnc_, hg::u32 ticks_) {
        hg::concurrent::promise<Ty> p { fnc_ };
        hg::concurrent::future<Ty> f = p.get();

        delay({ p }, ticks_);

        return f;
    }
}
