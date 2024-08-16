#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Core/Module/RootModule.hpp>

#include "Task/Task.hpp"

namespace hg::engine::scheduler {
	class CompositePipeline;
	class Stage;
}

namespace hg::engine {
	class macro_novtable Scheduler :
		public core::RootModule {
	public:
		using task_type = ::hg::engine::scheduler::task::TaskDelegate;
		using task_handle_type = non_owning_rptr<const task_type>;

		constexpr static inline u32 auto_worker_count { 0uL };

	protected:
		Scheduler() noexcept = default;

	public:
		virtual ~Scheduler() = default;

	public:
		/**
		 * Setups the given workers
		 *
		 * @author Julius
		 * @date 13.11.2020
		 *
		 * @param  workers_ The workers.
		 */
		virtual void setup(u32 workers_) = 0;

		virtual void finalize() = 0;

		virtual void destroy() = 0;

	public:
		/**
		 * Delays a task_ for ticks_
		 *
		 * @author Julius
		 * @date 13.11.2020
		 *
		 * @param [in] task_ The task.
		 * @param 	   ticks_ The ticks.
		 */
		virtual void delay(_In_ mref<task_handle_type> task_, const u32 ticks_) = 0;

		/**
		 * Executes the given task
		 *
		 * @author Julius
		 * @date 16.11.2020
		 *
		 * @param [in] task_ The task.
		 */
		virtual void exec(_In_ mref<task_handle_type> task_) = 0;

	public:
		[[nodiscard]] virtual const non_owning_rptr<scheduler::CompositePipeline> getCompositePipeline() const noexcept
		= 0;

	public:
		virtual bool execOnStage(
			_In_ mref<task_handle_type> task_,
			const non_owning_rptr<const scheduler::Stage> stage_
		) = 0;

		virtual bool execOnStages(
			_In_ mref<task_handle_type> task_,
			const non_owning_rptr<const scheduler::Stage> begin_,
			const non_owning_rptr<const scheduler::Stage> end_
		) = 0;

	public:
		/**
		 * Gets worker count
		 *
		 * @author Julius
		 * @date 13.11.2020
		 *
		 * @returns The worker count.
		 */
		[[nodiscard]] virtual size_t getWorkerCount() const = 0;

		/**
		 * Waits this 
		 *
		 * @author Julius
		 * @date 13.11.2020
		 */
		virtual void wait() const = 0;
	};
}
