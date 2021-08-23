#pragma once
#include <Engine.Common/Concurrent/Promise.hpp>
#include "Scheduler.hpp"

namespace ember::engine::scheduler {
    /**
     * Delays
     *
     * @author Julius
     * @date 16.11.2020
     *
     * @param [in] task_ The task.
     * @param 	   ticks_ The ticks.
     */
    void delay(IN task::__TaskDelegate task_, const ember::u32 ticks_);

    /**
     * Delays
     *
     * @author Julius
     * @date 16.11.2020
     *
     * @param [in] fnc_ The function.
     * @param 	   ticks_ The ticks.
     */
    void delay(IN task::Task::function_type&& fnc_, const ember::u32 ticks_);

    /**
     * Executes the given task
     *
     * @author Julius
     * @date 16.11.2020
     *
     * @param [in] task_ The task.
     */
    void exec(IN task::__TaskDelegate task_);

    /**
     * Executes the given function as task
     *
     * @author Julius
     * @date 16.11.2020
     *
     * @param [in] fnc_ The function.
     */
    void exec(IN task::Task::function_type&& fnc_);

    /**
     * Schedules the function and return a async resolved future
     *
     * @tparam Ty Type of the ty.
     * @param [in] fnc_ The function.
     *
     * @returns A concurrent::future&lt;Ty&gt;
     */
    template <typename Ty>
    ember::concurrent::future<Ty> async(IN _STD function<Ty()>&& fnc_) {
        using namespace ember::concurrent;
        promise<Ty> p { fnc_ };
        future<Ty> f = p.get();

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
    ember::concurrent::future<Ty> async(IN _STD function<Ty()>&& fnc_, ember::u32 ticks_) {
        using namespace ember::concurrent;
        promise<Ty> p { fnc_ };
        future<Ty> f = p.get();

        delay({ p }, ticks_);

        return f;
    }
}
