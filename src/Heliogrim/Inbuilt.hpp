#pragma once

#include <concepts>

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Set.hpp>
#include <Engine.Common/Concurrent/Future.hpp>
#include <Engine.Common/Functional/Function.hpp>
#include <Engine.Common/Math/__default.inl>
#include <Engine.Scheduler/Fiber/Awaitable.hpp>

#include "Future.hpp"
#include "Task.hpp"

namespace hg {
	/**
	 * Section: Basic Data Types
	 */
	using Color3 = math::fvec3;
	using Color4 = math::fvec4;

	using Mat2 = math::fmat2;
	using Mat3 = math::fmat3;
	using Mat4 = math::fmat4;

	using Quatern = math::fquaternion;

	using Transform = math::Transform;

	using Vector2 = math::fvec2;
	using Vector3 = math::fvec3;
	using Vector4 = math::fvec4;

	/**
	 * Section: Specific Data Types
	 */

	/**
	 * Subsection: Suspension
	 */
	using await_signal_type = engine::scheduler::fiber::await_signal_type;
	using await_signal_sub_type = engine::scheduler::fiber::await_signal_sub_type;

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

	using execute_function_type = std::function<void()>;
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

	/**
	 * Subsection: Suspension
	 */

	/**
	 * Awaits the current execution context until signal arise
	 *
	 * @author Julius
	 * @date 23.10.2021
	 *
	 * @param  signal_ The signal where to subscribe / wait for.
	 */
	extern void await(_In_ const ptr<await_signal_sub_type> signal_);

	/**
	 * Awaits the given future and returns the stored value
	 *
	 * @author Julius
	 * @date 26.11.2021
	 *
	 * @param future_ The future to wait for and resolved value from.
	 *
	 * @returns The stored value instance of the future.
	 */
	template <typename Type_>
	Type_&& await(_In_ mref<Future<Type_>> future_) {
		await(future_);
		return future_.get();
	}

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
	 * Try to remove the execution guarantee of this context if exists
	 *
	 * @author Julius
	 * @date 22.11.2021
	 *
	 * @note Could fail if context is marked with ctrl token
	 *
	 * @returns True if guarantee was removed, otherwise false.
	 */
	//extern bool desync();

	/**
	 * Try to sync this context to scheduling with given guarantees
	 *
	 * @author Julius
	 * @date 22.11.2021
	 *
	 * @node Could fail is context is marked with ctrl token
	 *
	 * @returns True if context enforced guarantees, otherwise false.
	 */
	//extern bool sync(_In_ cref<TaskStage> src_, _In_ cref<TaskStage> dst_);
}
