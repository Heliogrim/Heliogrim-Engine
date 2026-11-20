#pragma once

#include <Engine.Common/__macro.hpp>
#include <Engine.Common/Sal.hpp>
#include <Engine.Core/Module/RootModule.hpp>

#include "LanePolicyConfig.hpp"
#include "Task/Task.hpp"

namespace hg::engine::scheduler {
	class CompositePipeline;
	class Stage;
	struct BoostCell;
}

namespace hg::engine {
	class macro_novtable Scheduler :
		public core::RootModule {
	public:
		using task_type = ::hg::engine::scheduler::task::TaskDelegate;
		using task_handle_type = non_owning_rptr<const task_type>;

		constexpr static inline u32 auto_worker_count { 0uL };
		constexpr static inline u32 default_worker_reserve { 0uL };

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
		 * @param  workers_ The workers (0 derives the count from the reserve).
		 * @param  reserve_ Logical threads left to non-worker threads (0 selects the default).
		 * @param  lanes_   The lane policy (floors, lane reserve, Frame wait limit).
		 */
		virtual void setup(u32 workers_, u32 reserve_, ref<const scheduler::LanePolicyConfig> lanes_) = 0;

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

		/**
		 * Pushes the task with the lane it already carries; the lane was resolved where the task was created
		 */
		virtual void execResolved(_In_ mref<task_handle_type> task_) = 0;

		/**
		 * As execResolved, but queued in the higher of the task's lane and cell_'s lane; a boost of cell_ racing the push
		 * is not lost. cell_ must outlive the task (the task holds it).
		 */
		virtual void execBoostable(_In_ mref<task_handle_type> task_, _In_ ptr<scheduler::BoostCell> cell_) = 0;

		/**
		 * Executes the given task with a cooperative cancellation token
		 */
		void exec(_In_ mref<task_handle_type> task_, scheduler::fiber::CancellationToken token_) {
			// Handles are const by contract; the token is set before the task is scheduled.
			const_cast<ptr<task_type>>(task_)->cancellation() = std::move(token_);
			exec(std::move(task_));
		}

	public:
		[[nodiscard]] virtual const non_owning_rptr<scheduler::CompositePipeline> getCompositePipeline() const noexcept = 0;

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
		 * Tasks queued in lane_, not yet popped.
		 */
		[[nodiscard]] virtual u64 queued(const scheduler::Lane lane_) const = 0;

		/**
		 * Waits this 
		 *
		 * @author Julius
		 * @date 13.11.2020
		 */
		virtual void wait() const = 0;
	};
}
