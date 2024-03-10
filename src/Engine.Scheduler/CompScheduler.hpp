#pragma once

#include <Engine.Common/Sal.hpp>

#include "Scheduler.hpp"
#include "Process/Schedule.hpp"
#include "Pipeline/CompositePipeline.hpp"
#include "Worker/Worker.hpp"

namespace hg::engine {
	class CompScheduler /* TODO: final */ :
		public Scheduler {
	public:
		using aligned_worker = ALIGNED(scheduler::worker::Worker, CACHE_LINE_SIZE);
		using fiber_pool_type = scheduler::fiber::FiberPool;

		using task_type = ::hg::engine::scheduler::task::TaskDelegate;

	public:
		/**
		 * Default constructor
		 *
		 * @author Julius
		 * @date 14.11.2020
		 */
		CompScheduler() noexcept;

		/**
		 * Destructor
		 *
		 * @author Julius
		 * @date 13.11.2020
		 */
		~CompScheduler() override;

	public:
		/**
		 * Setups the given workers
		 *
		 * @author Julius
		 * @date 13.11.2020
		 *
		 * @param  workers_ The workers.
		 */
		void setup(u32 workers_) override;

		void finalize() override;

		void destroy() override;

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
		void delay(_In_ mref<task_handle_type> task_, const u32 ticks_) override;

		/**
		 * Executes the given task
		 *
		 * @author Julius
		 * @date 16.11.2020
		 *
		 * @param [in] task_ The task.
		 */
		void exec(_In_ mref<task_handle_type> task_) override;

	public:
		[[nodiscard]] const non_owning_rptr<scheduler::CompositePipeline>
		getCompositePipeline() const noexcept override;

	public:
		bool execOnStage(
			mref<task_handle_type> task_,
			const non_owning_rptr<const scheduler::Stage> stage_
		) override;

		bool execOnStages(
			mref<task_handle_type> task_,
			const non_owning_rptr<const scheduler::Stage> begin_,
			const non_owning_rptr<const scheduler::Stage> end_
		) override;

		/**
		 * Gets worker count
		 *
		 * @author Julius
		 * @date 13.11.2020
		 *
		 * @returns The worker count.
		 */
		[[nodiscard]] size_t getWorkerCount() const override;

		/**
		 * Waits this 
		 *
		 * @author Julius
		 * @date 13.11.2020
		 */
		void wait() const override;

	private:
		u32 _workerCount;
		aligned_worker* _workers;

	private:
		/** The fiber pool */
		fiber_pool_type _fiberPool;

	protected:
		uptr<scheduler::CompositePipeline> _pipeline;
		uptr<scheduler::Schedule> _schedule;
	};
}
