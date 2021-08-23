#pragma once
#include "Task/Task.hpp"
#include "Thread/Worker.hpp"

namespace ember::engine::scheduler {
    class Scheduler final {
    private:
        using aligned_worker = ALIGNED(thread::Worker, CACHE_LINE_SIZE);

        /**
         * Default constructor
         *
         * @author Julius
         * @date 14.11.2020
         */
        Scheduler() noexcept;

    public:
        /**
         * Destructor
         *
         * @author Julius
         * @date 13.11.2020
         */
        ~Scheduler();

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
        void exec(IN task::__TaskDelegate task_);

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
         * Create a instance of scheduler and stores it's global static instance
         *
         * @author Julius
         * @date 16.11.2020
         *
         * @returns A reference to a Scheduler.
         */
        static Scheduler& make();

        /**
         * Destroys this and flushes static internal stored instance
         *
         * @author Julius
         * @date 22.07.2021
         */
        static void destroy() noexcept;

    private:
        /** The shared tasks */
        task::SharedQueue _sharedTasks;

        u32 _workerCount;
        aligned_worker* _workers;

        /** The instance */
        static Scheduler* _instance;
    };
}
