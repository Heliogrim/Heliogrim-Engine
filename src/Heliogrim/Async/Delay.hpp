#pragma once

#include <Engine.Common/Sal.hpp>

#include "ExecFn.hpp"
#include "Tick.hpp"

namespace hg {
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
}
