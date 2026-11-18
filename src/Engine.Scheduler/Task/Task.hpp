#pragma once

#include <chrono>
#include <functional>
#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "TaskCtrl.hpp"
#include "../Lane.hpp"
#include "TaskType.hpp"
#include "../Fiber/Cancellation.hpp"

namespace hg::engine::scheduler::fiber {
	struct Fiber;
}

namespace hg::engine::scheduler::task {
	/**
	 * ! Important !
	 *
	 * A Task of any [type] with any [lane] should be allocated on a dense head block of tasks to improve cache delay
	 * A Task of any [type] should be based on internal type [TaskDelegate]
	 * A Task will be invoked directly without type check of via [TaskDelegate] and get downcast internal
	 */

	class TaskDelegate {
	public:
		using this_type = TaskDelegate;

	protected:
		/**
		 * Constructor
		 *
		 * @author Julius
		 * @date 16.11.2020
		 *
		 * @param  type_ The type.
		 * @param  lane_ The lane.
		 */
		TaskDelegate(const TaskType type_, const Lane lane_) noexcept;

	public:
		TaskDelegate(const TaskDelegate&) = delete;

		TaskDelegate(TaskDelegate&&) noexcept = delete;

		/**
		 * Destructor
		 *
		 * @author Julius
		 * @date 16.11.2020
		 */
		~TaskDelegate() = default;

	public:
		ref<TaskDelegate> operator=(cref<TaskDelegate>) = delete;

		ref<TaskDelegate> operator=(mref<TaskDelegate>) = delete;

	public:
		/**
		 * Will downcast to the concrete type and execute via static cast
		 *
		 * @author Julius
		 * @date 16.11.2020
		 */
		void delegate() const;

	protected:
		/**
		 * Type definition of the task
		 */

		/** The type */
		const TaskType _type;

		/** The lane */
		Lane _lane;

	public:
		/** When the schedule last queued this task; stamped by Schedule::push for Frame tasks only. */
		mutable std::chrono::steady_clock::time_point queuedAt {};

	public:
		/**
		 * Gets the type
		 *
		 * @author Julius
		 * @date 13.11.2020
		 *
		 * @returns A TaskType.
		 */
		[[nodiscard]] TaskType type() const noexcept;

		/**
		 * Gets the lane
		 *
		 * @author Julius
		 * @date 15.11.2020
		 *
		 * @returns A Lane.
		 */
		[[nodiscard]] Lane lane() const noexcept;

		[[nodiscard]] ref<Lane> lane() noexcept;

	protected:
		/**
		 * Execution state of the task
		 */

		/** The fiber */
		ptr<fiber::Fiber> _fiber;

	public:
		/**
		 * Gets the fiber
		 *
		 * @author Julius
		 * @date 22.10.2021
		 *
		 * @returns A ptr&lt;fiber::Fiber&gt;
		 */
		[[nodiscard]] ptr<fiber::Fiber> fiber() const noexcept;

		/**
		 * Gets the fiber
		 *
		 * @author Julius
		 * @date 22.10.2021
		 *
		 * @returns A ref&lt;ptr&lt;fiber::Fiber&gt;&gt;
		 */
		[[nodiscard]] ref<ptr<fiber::Fiber>> fiber() noexcept;

	private:
		ptr<TaskCtrl> _ctrl;

	public:
		[[nodiscard]] ptr<TaskCtrl> ctrl() const noexcept;

		[[nodiscard]] ref<ptr<TaskCtrl>> ctrl() noexcept;

	private:
		fiber::CancellationToken _cancellation;

	public:
		[[nodiscard]] ref<const fiber::CancellationToken> cancellation() const noexcept {
			return _cancellation;
		}

		[[nodiscard]] ref<fiber::CancellationToken> cancellation() noexcept {
			return _cancellation;
		}
	};

	class Task :
		public TaskDelegate {
	public:
		using function_type = std::function<void()>;

		/**
		 * Destructor
		 *
		 * @author Julius
		 * @date 13.11.2020
		 */
		~Task() noexcept;

		/**
		 * Executes this wrapped operation
		 *
		 * @author Julius
		 * @date 16.11.2020
		 */
		void invoke() const;

	private:
		/**
		 * The wrapped function
		 */
		function_type _fnc;

		friend const non_owning_rptr<const Task> make_task(function_type&&, Lane);

		/**
		 * Constructor
		 *
		 * @author Julius
		 * @date 16.11.2020
		 *
		 * @param fnc_ The function.
		 * @param lane_ The lane.
		 */
		Task(_In_ function_type&& fnc_, const Lane lane_) noexcept;
	};

	/**
	 * Make Constructor for [Task]
	 *
	 * @author Julius
	 * @date 16.11.2020
	 *
	 * @param fnc_ The function.
	 * @param lane_ (Optional) The lane.
	 */
	const non_owning_rptr<const Task> make_task(
		_In_ Task::function_type&& fnc_,
		const Lane lane_ = Lane::eInherit
	);

	class RepetitiveTask :
		public TaskDelegate {
	public:
		using function_type = std::function<bool()>;

		/**
		 * Destructor
		 *
		 * @author Julius
		 * @date 14.11.2020
		 */
		~RepetitiveTask() noexcept;

		/**
		 * Executes this wrapper operation
		 *
		 * @author Julius
		 * @date 16.11.2020
		 *
		 * @returns True if it succeeds, false if it fails.
		 */
		[[nodiscard]] bool invoke() const;

	private:
		/**
		 * The wrapped function
		 */
		function_type _fnc;

		friend const non_owning_rptr<const RepetitiveTask> make_repetitive_task(function_type&&, Lane);

		/**
		 * Constructor
		 *
		 * @author Julius
		 * @date 16.11.2020
		 *
		 * @param fnc_ The function.
		 * @param lane_ The lane.
		 */
		RepetitiveTask(_In_ function_type&& fnc_, const Lane lane_) noexcept;
	};

	/**
	 * Make Constructor for [RepetitiveTask]
	 *
	 * @author Julius
	 * @date 16.11.2020
	 *
	 * @param fnc_ The function.
	 * @param lane_ (Optional) The lane.
	 */
	const non_owning_rptr<const RepetitiveTask> make_repetitive_task(
		_In_ RepetitiveTask::function_type&& fnc_,
		const Lane lane_ = Lane::eInherit
	);

	class BatchTask :
		public TaskDelegate {
	public:
		using function_type = std::function<void(hg::u32)>;

		/**
		 * Destructor
		 *
		 * @author Julius
		 * @date 14.11.2020
		 */
		~BatchTask() noexcept;

		/**
		 * Executes this wrapper operation
		 *
		 * @author Julius
		 * @date 16.11.2020
		 */
		void invoke() const;

	private:
		/**
		 * The wrapped function
		 */
		function_type _fnc;

		friend const non_owning_rptr<const BatchTask> make_batch_task(function_type&&, Lane);

		/**
		 * Constructor
		 *
		 * @author Julius
		 * @date 16.11.2020
		 *
		 * @param fnc_ The function.
		 * @param lane_ The lane.
		 */
		BatchTask(_In_ function_type&& fnc_, const Lane lane_) noexcept;
	};

	/**
	 * Make Constructor for [BatchTask]
	 *
	 * @author Julius
	 * @date 16.11.2020
	 *
	 * @param fnc_ The function.
	 * @param lane_ (Optional) The lane.
	 */
	const non_owning_rptr<const BatchTask> make_batch_task(
		_In_ BatchTask::function_type&& fnc_,
		const Lane lane_ = Lane::eInherit
	);
}
