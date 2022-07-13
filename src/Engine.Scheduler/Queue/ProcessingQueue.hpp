#pragma once

#include "AtomicCtrlBlockPage.hpp"
#include "DynamicBufferPages.hpp"
#include "ProcessingQueueGuard.hpp"
#include "../Thread/Thread.hpp"
#include "SharedBufferPool.hpp"

namespace ember::engine::scheduler {

    class ProcessingQueue {
    public:
        using aligned_buffer = SharedBufferPool::aligned_buffer;
        using slot_type = ProcessingQueueGuard;
        using slot_page_type = ptr<slot_type>;

        using size_type = uint_fast16_t;
        using atomic_size_type = _STD atomic_uint_fast16_t;

        inline static constexpr size_type page_size = 16ui16;

    public:
        ProcessingQueue(_In_ const ptr<SharedBufferPool> pool_) noexcept;

        /**
         * Copy Constructor
         *
         * @author Julius
         * @date 12.11.2021
         */
        ProcessingQueue(cref<ProcessingQueue>) = delete;

        /**
         * Move Constructor
         *
         * @author Julius
         * @date 12.11.2021
         */
        ProcessingQueue(mref<ProcessingQueue>) noexcept = delete;

        ~ProcessingQueue();

    private:
        void tidy();

    public:
        /**
         * Setup this processing queue optimized for given parameters
         *
         * @author Julius
         * @date 17.11.2021
         *
         * @param workerCount_ The amount of workers concurrently accessing this.
         * @param maxSharedTasks_ The maximum amount of unconstrained tasks to handle.
         */
        void setup(_In_ const u64 workerCount_, _In_ const u64 maxSharedTasks_ = 496ui64);

    private:
        ptr<SharedBufferPool> _pool;

    private:
        AtomicCtrlBlockPage<aligned_buffer, page_size> _sharedCtrlBlockPage;

    private:
        _STD atomic<ptr<AtomicCtrlBlock<DynamicBufferPages>>> _currentPage;
        AtomicCtrlBlock<DynamicBufferPages> _pageCtrl[2];

    private:
        _STD atomic_flag _resizing;

    private:
        void grow();

    public:
        /**
         * Pushed a task without constrains to shared task queues
         *
         * @author Julius
         * @date 17.11.2021
         *
         * @param task_ The task to handle.
         */
        void push(_Inout_ mref<task::__TaskDelegate> task_);

        void pushStaged(_Inout_ mref<ptr<aligned_buffer>> buffer_);

    private:
        // void erase(_In_ const size_type hint_, _In_ const ptr<aligned_buffer> buffer_);

    public:
        void pop(_In_ const task::TaskMask mask_, _Inout_ ref<task::__TaskDelegate> task_) noexcept;
    };
}
