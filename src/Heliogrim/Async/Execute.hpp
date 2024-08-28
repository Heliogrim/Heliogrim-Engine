#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "ExecFn.hpp"
#include "Task.hpp"
#include "Tick.hpp"

namespace hg {
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
	 * Executes the given task asynchronously
	 *
	 * @author Julius
	 * @date 23.10.2021
	 *
	 * @param  task_ The task to execute.
	 */
	extern void execute(_Inout_ mref<Task> task_);

	/**
	 * Executes the given repetitive task asynchronously
	 *
	 * @author Julius
	 * @date 23.10.2021
	 *
	 * @param  task_ The task to execute repetitively.
	 */
	extern void execute(_Inout_ mref<RepetitiveTask> task_);

	/**
	 * Executes the given tick task asynchronously
	 *
	 * @author Julius
	 * @date 23.10.2021
	 *
	 * @param  task_ The task to execute tick based.
	 */
	extern void execute(_Inout_ mref<TickTask> task_);
}
