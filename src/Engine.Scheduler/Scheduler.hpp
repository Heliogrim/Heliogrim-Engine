#pragma once
#include "Queue/SchedulePipeline.hpp"
#include "Task/SignaledQueue.hpp"
#include "Task/Task.hpp"
#include "Worker/Worker.hpp"

namespace ember::engine {
    class Scheduler final {
    public:
        using aligned_worker = ALIGNED(scheduler::worker::Worker, CACHE_LINE_SIZE);
        using fiber_pool_type = scheduler::fiber::FiberPool;

        constexpr static inline u32 auto_worker_count { 0ui32 };

    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 14.11.2020
         */
        Scheduler() noexcept;

        /**
         * Destructor
         *
         * @author Julius
         * @date 13.11.2020
         */
        ~Scheduler();

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
        void delay(IN scheduler::task::__TaskDelegate task_, const u32 ticks_);

        /**
         * Executes the given task
         *
         * @author Julius
         * @date 16.11.2020
         *
         * @param [in] task_ The task.
         */
        void exec(_Inout_ scheduler::task::__TaskDelegate task_);

        /**
         * Gets worker count
         *
         * @author Julius
         * @date 13.11.2020
         *
         * @returns The worker count.
         */
        [[nodiscard]] size_t getWorkerCount() const;

        /**
         * Setups the given workers
         *
         * @author Julius
         * @date 13.11.2020
         *
         * @param  workers_ The workers.
         */
        void setup(u32 workers_);

        void destroy();

        /**
         * Waits this 
         *
         * @author Julius
         * @date 13.11.2020
         */
        void wait() const;

    private:
        scheduler::SchedulePipeline _pipeline;

        u32 _workerCount;
        aligned_worker* _workers;

    private:
        /** The fiber pool */
        fiber_pool_type _fiberPool;
    };
}
