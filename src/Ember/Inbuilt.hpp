#pragma once

#include <Engine.Common/String.hpp>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Functional/Function.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Scheduler/Fiber/Awaitable.hpp>

namespace ember {

    /**
     * Section: Basic Data Types
     */

    /**
     * Section: Specific Data Types
     */

    /**
     * Subsection: Ticks
     */

    /**
     * The basic data type for ticks to enforce type safety
     *
     * @author Julius
     * @date 23.10.2021
     */
    struct ticks {
        u64 unwrap;
    };

    /**
     * Section: Asynchronous Control Flow
     */

    /**
     * Subsection: Dispatching
     */

    using execute_function_type = _STD function<void()>;
    using execute_function_pointer_type = nular_fnc<void>;

    /**
     * Executes the given function delayed by given ticks as task asynchronously
     *
     * @author Julius
     * @date 23.10.2021
     *
     * @param  function_ The function to execute.
     * @param  delay_ The delay in ticks.
     */
    extern void delay(_Inout_ mref<execute_function_type> function_, _In_ ticks delay_);

    /**
     * Executes the given function delayed by given ticks as task asynchronously
     *
     * @author Julius
     * @date 23.10.2021
     *
     * @param  function_ The function to execute.
     * @param  delay_ The delay in ticks.
     */
    extern void delay(_In_ execute_function_pointer_type function_, _In_ ticks delay_);

    /**
     * Executes the given function as task asynchronously
     *
     * @author Julius
     * @date 23.10.2021
     *
     * @param  function_ The function to execute.
     */
    extern void execute(_Inout_ mref<execute_function_type> function_);

    /**
     * Executes the given function as task asynchronously
     *
     * @author Julius
     * @date 23.10.2021
     *
     * @param  function_ The function to execute.
     */
    extern void execute(_In_ execute_function_pointer_type function_);

    /**
     * Subsection: Suspension
     */

    using await_signal_type = engine::scheduler::fiber::await_signal_type;

    /**
     * Yields the current execution context at least for given milliseconds
     *
     * @author Julius
     * @date 23.10.2021
     *
     * @param  milliseconds_ The milliseconds suspended at least.
     */
    extern void sleep(_In_ const u64 milliseconds_);

    /**
     * Yields the current execution context
     *
     * @author Julius
     * @date 23.10.2021
     */
    extern void yield();

    /**
     * Awaits the current execution context until signal arise
     *
     * @author Julius
     * @date 23.10.2021
     *
     * @param  signal_ The signal to wait for.
     */
    extern void await(_In_ const ptr<await_signal_type> signal_);
}
