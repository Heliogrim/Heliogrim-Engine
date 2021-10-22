#pragma once
#ifdef _DEBUG
#include <assert.h>
#endif

#include <Engine.Common/__macro.hpp>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Concurrent/Collection/RingBuffer.hpp>
#include <Engine.Common/Memory/allocator.hpp>

#include "Task.hpp"
#include "../Thread/Thread.hpp"

namespace ember::engine::scheduler {
    namespace worker {
        /**
         * Forward Declaration
         */
        class Worker;
    }

    namespace task {

        struct SharedSubQueue {

            /**
             * Constructor
             *
             * @author Julius
             * @date 15.12.2020
             *
             * @param  mask_ The mask.
             */
            SharedSubQueue(TaskMask mask_) noexcept;

            const TaskMask mask;

            _STD atomic<thread::thread_id> owner { 0 };

            concurrent::RingBuffer<__TaskDelegate> buffer;

            /**
             * Copy Constructor
             *
             * @author Julius
             * @date 15.12.2020
             */
            SharedSubQueue(const SharedSubQueue&) = delete;

            /**
             * Move Constructor
             *
             * @author Julius
             * @date 15.12.2020
             */
            SharedSubQueue(SharedSubQueue&&) = delete;

            /**
             * Attempts to acquire
             *
             * @author Julius
             * @date 16.11.2020
             *
             * @returns True if it succeeds, false if it fails.
             */
            [[nodiscard]] bool try_acquire();

            /**
             * Acquires this 
             *
             * @author Julius
             * @date 16.11.2020
             */
            void acquire();

            /**
             * Releases this 
             *
             * @author Julius
             * @date 16.11.2020
             */
            void release();
        };

        class SharedQueue {
            friend class ::ember::engine::scheduler::worker::Worker;

            using aligned_subqueue = ALIGNED(SharedSubQueue, CACHE_LINE_SIZE);

        public:
            /**
             * Constructor
             *
             * @author Julius
             * @date 15.11.2020
             */
            SharedQueue();

            /**
             * Destructor
             *
             * @author Julius
             * @date 14.11.2020
             */
            ~SharedQueue();

            /**
             * Setups queue for given thread count
             *
             * @author Julius
             * @date 16.11.2020
             *
             * @param  threads_ The threads.
             */
            void setup(u32 threads_);

            /**
             * Destroys this 
             *
             * @author Julius
             * @date 16.11.2020
             */
            void destroy();

            /**
             * Pushes an object onto this queue
             *
             * @author Julius
             * @date 15.12.2020
             *
             * @param [in,out] task_ The task to push.
             */
            void push(__TaskDelegate&& task_);

            /**
             * Pops an object from this queue
             *
             * @author Julius
             * @date 15.12.2020
             *
             * @param          mask_ The mask to pop.
             * @param [out] result_ The result.
             */
            void pop(TaskMask mask_, OUT __TaskDelegate& result_) noexcept;

        private:
            /**
             * SharedSubQueue's
             */
            aligned_subqueue* _queues;
            u8 _queueCount;
        };
    }
}
