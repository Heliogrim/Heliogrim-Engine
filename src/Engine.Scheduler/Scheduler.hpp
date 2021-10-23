#pragma once
#include "Task/SignaledQueue.hpp"
#include "Task/Task.hpp"
#include "Worker/Worker.hpp"

namespace ember::engine::scheduler {
    class Scheduler final {
    private:
        using aligned_worker = ALIGNED(worker::Worker, CACHE_LINE_SIZE);
        using fiber_pool_type = fiber::FiberPool;

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

    private:
        /** The instance */
        static ptr<Scheduler> _instance;

    public:
        /**
         * Gets the instance
         *
         * @author Julius
         * @date 13.11.2020
         *
         * @returns A reference to a Scheduler.
         */
        [[nodiscard]] static Scheduler& get();

        /**
         * Gets the singleton instance pointer
         *
         * @author Julius
         * @date 13.10.2021
         *
         * @returns A pointer to a Scheduler, otherwise nullptr
         */
        [[nodiscard]] static ptr<Scheduler> get(_STD nothrow_t) noexcept;

        /**
         * Create a instance of scheduler and stores it's global static instance
         *
         * @author Julius
         * @date 16.11.2020
         *
         * @returns A pointer to a Scheduler.
         */
        static ptr<Scheduler> make();

        /**
         * Destroys this and flushes static internal stored instance
         *
         * @author Julius
         * @date 22.07.2021
         */
        static void destroy() noexcept;

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
        void delay(IN task::__TaskDelegate task_, const u32 ticks_);

        /**
         * Executes the given task
         *
         * @author Julius
         * @date 16.11.2020
         *
         * @param [in] task_ The task.
         */
        void exec(_Inout_ task::__TaskDelegate task_);

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
         * Tidy up
         *
         * @author Julius
         * @date 22.07.2021
         */
        void tidy();

        /**
         * Setups the given workers
         *
         * @author Julius
         * @date 13.11.2020
         *
         * @param  workers_ The workers.
         */
        void setup(u32 workers_);

        /**
         * Waits this 
         *
         * @author Julius
         * @date 13.11.2020
         */
        void wait() const;

    private:
        /** The shared tasks */
        task::SharedQueue _sharedTasks;

        u32 _workerCount;
        aligned_worker* _workers;

    private:
        /** The fiber pool */
        fiber_pool_type _fiberPool;
    };
}
