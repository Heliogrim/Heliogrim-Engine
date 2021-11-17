#pragma once

#include <Engine.Common/__macro.hpp>
#include "SharedBufferPool.hpp"

namespace ember::engine::scheduler {

    class ProcessingQueueGuard {
    public:
        using aligned_buffer = SharedBufferPool::aligned_buffer;

    public:
        ProcessingQueueGuard() = delete;

        ProcessingQueueGuard(_In_ const ptr<SharedBufferPool> pool_) noexcept;

        ProcessingQueueGuard(cref<ProcessingQueueGuard>) = delete;

        ProcessingQueueGuard(mref<ProcessingQueueGuard>) = delete;

        ~ProcessingQueueGuard();

    private:
        _STD atomic_flag _pLck;
        _STD atomic_flag _cLck;

    private:
        ptr<SharedBufferPool> _pool;
        ptr<aligned_buffer> _buffer;

    private:
        /**
         * Will acquire a buffer for this usage
         *
         * @author Julius
         * @date 14.11.2021
         */
        void acquire();

        /**
         * Will release this buffer and reset constraints
         *
         * @author Julius
         * @date 13.11.2021
         */
        void release();

    public:
        /**
         *
         * @author Julius
         * @date 13.11.2021
         *
         * @param buffer_ The buffer to store into this guard to handle.
         *
         * @returns True if succeeded without waiting, otherwise false
         */
        [[nodiscard]] _Success_(return == true) bool try_store(_Inout_ mref<ptr<aligned_buffer>> buffer_) noexcept;

        /**
         *
         * @author Julius
         * @date 13.11.2021
         *
         * @param buffer_ The buffer to store into this guard to handle.
         *
         * @returns True is succeeded even with waiting, otherwise false
         */
        [[nodiscard]] _Success_(return == true) bool store(_Inout_ mref<ptr<aligned_buffer>> buffer_) noexcept;

    public:
        /**
         * Try to push a task onto this queue
         *
         * @author Julius
         * @date 13.11.2021
         *
         * @param task_ The task to push onto this.
         *
         * @returns True if succeeded, otherwise false.
         */
        [[nodiscard]] _Success_(return == true) bool try_push(_Inout_ mref<task::__TaskDelegate> task_) noexcept;

        /**
         * Try to pop a task from this queue
         *
         * @author Julius
         * @date 13.11.2021
         *
         * @param mask_ The mask to check against tasks.
         * @param task_ The reference where to pop the task to.
         *
         * @returns True if succeeded, otherwise false.
         */
        [[nodiscard]] _Success_(return == true) bool try_pop(_In_ const task::TaskMask mask_,
            _Out_ ref<task::__TaskDelegate> task_) noexcept;
    };
}
