#pragma once

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

    private:
        ptr<SharedBufferPool> _pool;

    private:
        _STD atomic<ptr<slot_page_type>> _pages;
        atomic_size_type _pageCount;

    public:
        [[nodiscard]] cref<atomic_size_type> pageCount() const noexcept;

    private:
        _STD atomic<thread::thread_id> _resizing;

    private:
        void grow();

    public:
        void push(_Inout_ mref<task::__TaskDelegate> task_);

        void pushStaged(_Inout_ mref<ptr<aligned_buffer>> buffer_);

    private:
        // void erase(_In_ const size_type hint_, _In_ const ptr<aligned_buffer> buffer_);

    public:
        void pop(_In_ const task::TaskMask mask_, _Inout_ ref<task::__TaskDelegate> task_) noexcept;
    };
}
