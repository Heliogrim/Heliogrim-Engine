#include "ProcessingQueue.hpp"

#include <iostream>

using namespace ember::engine::scheduler;
using namespace ember;

/**
 * Attention:
 *  This processing queue should store new tasks into bottom most queues, using bottom - up iteration
 *  This processing queue should pop tasks from top most queues, using top - down iteration
 *
 *  Using these two constraints, will cause a release of empty queues at the top and compact task management at the bottom
 */

ProcessingQueue::ProcessingQueue(const ptr<SharedBufferPool> pool_) noexcept :
    _pool(pool_),
    _pages(nullptr),
    _pageCount(0),
    _resizing() {}

ProcessingQueue::~ProcessingQueue() {
    tidy();
}

void ProcessingQueue::tidy() {

    if (_pages.load(_STD memory_order_relaxed) != nullptr) {
        return;
    }

    auto* pages = _pages.load(_STD memory_order_relaxed);
    auto count = _pageCount.load(_STD memory_order_relaxed);

    for (size_type i = 0; i < count; ++i) {
        delete[] pages[i];
    }

    free(pages);

    _pages.store(nullptr, _STD memory_order_relaxed);
    _pageCount.store(0, _STD memory_order_relaxed);
}

cref<ProcessingQueue::atomic_size_type> ProcessingQueue::pageCount() const noexcept {
    return _pageCount;
}

void ProcessingQueue::grow() {

    /**
     * Acquire
     */
    const auto threadId { thread::self::getId() };
    thread::thread_id expect { 0 };
    while (!_resizing.compare_exchange_weak(expect, threadId, _STD memory_order_release,
        _STD memory_order_relaxed)) {
        expect = 0;// Spinning
    }

    auto* const pages = _pages.load(_STD memory_order_consume);
    const size_type count = _pageCount.load(_STD memory_order_consume);

    auto newCount = count + 1;
    auto* newPages { static_cast<ptr<slot_page_type>>(calloc(newCount, sizeof(slot_page_type))) };

    /**
     * Translocate old pages
     */
    for (size_type i = 0; i < count; ++i) {
        newPages[i] = pages[i];
    }

    /**
     * Create new page
     */
    auto* page { static_cast<slot_page_type>(calloc(page_size, sizeof(slot_type))) };
    for (size_type i = 0; i < page_size; ++i) {
        new(&page[i]) slot_type { _pool };

    }

    newPages[count] = page;

    /**
     * Swap resources
     */
    _pages.store(newPages, _STD memory_order_release);
    _pageCount.store(newCount, _STD memory_order_release);

    /**
     * Release
     */
    _resizing.store(0, _STD memory_order_release);

    /**
     * Free old resources
     */
    free(pages);
}

void ProcessingQueue::push(mref<task::__TaskDelegate> task_) {

    while (true) {
        /**
         * Loop over every available queue, but enforce valid index range
         */
        const auto available = _pageCount.load(_STD memory_order_relaxed);
        for (size_type i = 0; i < available; ++i) {

            // TODO: Eliminate race condition...
            const auto threadId { thread::self::getId() };
            thread::thread_id expect { 0 };
            while (!_resizing.compare_exchange_weak(expect, threadId, _STD memory_order_release,
                _STD memory_order_relaxed)) {
                expect = 0;// Spinning
            }

            const auto* const pages = _pages.load(_STD memory_order_consume);
            auto* const page { pages[i] };

            _resizing.store(0, _STD memory_order_release);

            /**
             * Try to use current page
             */
            for (size_type j = 0; j < page_size; ++j) {

                if (page[j].try_push(_STD move(task_))) {
                    return;
                }

            }
        }

        /**
         * Failed to push task to current available slots
         */
        grow();
    }
}

void ProcessingQueue::pushStaged(mref<ptr<aligned_buffer>> buffer_) {

    /**
     * First try pushing stage without waiting
     */
    {
        /**
         * Loop over every available queue, but enforce valid index range
         */
        const auto available = _pageCount.load(_STD memory_order_relaxed);
        for (size_type i = 0; i < available; ++i) {

            // TODO: Eliminate race condition...
            const auto threadId { thread::self::getId() };
            thread::thread_id expect { 0 };
            while (!_resizing.compare_exchange_weak(expect, threadId, _STD memory_order_release,
                _STD memory_order_relaxed)) {
                expect = 0;// Spinning
            }

            const auto* const pages = _pages.load(_STD memory_order_consume);
            auto* const page { pages[i] };

            _resizing.store(0, _STD memory_order_release);

            /**
             * Try to use current page
             */
            for (size_type j = 0; j < page_size; ++j) {

                if (page[j].try_store(_STD move(buffer_))) {
                    return;
                }

            }
        }
    }

    /**
     * Force pushing stage
     */
    while (true) {
        /**
         * Loop over every available queue, but enforce valid index range
         */
        const auto available = _pageCount.load(_STD memory_order_relaxed);
        for (size_type i = 0; i < available; ++i) {

            // TODO: Eliminate race condition...
            const auto threadId { thread::self::getId() };
            thread::thread_id expect { 0 };
            while (!_resizing.compare_exchange_weak(expect, threadId, _STD memory_order_release,
                _STD memory_order_relaxed)) {
                expect = 0;// Spinning
            }

            const auto* const pages = _pages.load(_STD memory_order_consume);
            auto* const page { pages[i] };

            _resizing.store(0, _STD memory_order_release);

            /**
             * Try to use current page
             */
            for (size_type j = 0; j < page_size; ++j) {

                if (page[j].store(_STD move(buffer_))) {
                    return;
                }

            }
        }

        /**
         * Failed to push task to current available slots
         */
        grow();
    }

}

#if FALSE
void ProcessingQueue::erase(const size_type hint_, const ptr<aligned_queue> queue_) {

    /**
     * Acquire
     */
    const auto threadId { thread::self::getId() };
    thread::thread_id expect { 0 };
    while (!_resizing.compare_exchange_weak(expect, threadId, _STD memory_order_release, _STD memory_order_relaxed)) {
        expect = 0;// Spinning
    }

    /**
     * Check whether hint is valid and matching
     */
    const auto* const queues = _queues.load(_STD memory_order_relaxed);
    if (
        hint_ < _queueCount.load(_STD memory_order_relaxed) && queues[hint_] == queue_
    ) {

        /**
         * Replace in structure
         */
        const auto size = _queueCount.load(_STD memory_order_relaxed);
        if (size > 0 && hint_ < (size - 1)) {
            _queues[hint_] = _queues[size - 1];
            _queues[size - 1] = nullptr;
        } else {
            _queues[0] = nullptr;
        }

        DEBUG_ASSERT(size > 0, "Failed to evaluate queue count correctly. Possibly caused by race condition...")
        if (size < 1) {
            throw _STD runtime_error("");
        }
        std::cout << thread::self::getId() << " | Dec (1)" << std::endl;
        thread::self::yield();
        --_queueCount;

    } else {

        /**
         * Find queue to erase
         */
        const size_type size = _queueCount.load(_STD memory_order_relaxed);

        auto ci = size;
        for (; ci > 0 && _queues[ci - 1] == queue_; --ci) { }

        // Warning: Could possible corrupt state if queue to erase does not exist anymore

        /**
         * Replace in structure
         */
        if (size > 0 && ci > 0 && ci < size) {
            _queues[ci - 1] = _queues[size - 1];
            _queues[size - 1] = nullptr;
        } else {
            _queues[0] = nullptr;
        }

        DEBUG_ASSERT(size > 0, "Failed to evaluate queue count correctly. Possibly caused by race condition...")
        if (size < 1) {
            throw _STD runtime_error("");
        }
        std::cout << thread::self::getId() << " | Dec (2)" << std::endl;
        thread::self::yield();
        --_queueCount;
    }

    /**
     * Release queue
     */
    std::cout << thread::self::getId() << " | Release " << queue_ << std::endl;
    thread::self::yield();
    _pool->release(_STD move(const_cast<ptr<aligned_queue>>(queue_)));

    /**
     * Release
     */
    _resizing.store(0, _STD memory_order_release);
}
#endif

void ProcessingQueue::pop(const task::TaskMask mask_, ref<task::__TaskDelegate> task_) noexcept {

    /**
     * Loop over every available queue, but enforce valid index range
     */
    for (
        size_type ci = _pageCount.load(_STD memory_order_relaxed);
        ci <= _pageCount.load(_STD memory_order_relaxed) && ci > 0;
        --ci
    ) {

        // TODO: Eliminate race condition...
        const auto threadId { thread::self::getId() };
        thread::thread_id expect { 0 };
        while (!_resizing.compare_exchange_weak(expect, threadId, _STD memory_order_release,
            _STD memory_order_relaxed)) {
            expect = 0;// Spinning
        }

        const auto* const pages = _pages.load(_STD memory_order_consume);
        auto* const page { pages[ci - 1] };

        _resizing.store(0, _STD memory_order_release);

        /**
         * Try to use current page
         */
        for (size_type i = 0; i < page_size; ++i) {

            if (page[i].try_pop(mask_, task_)) {
                return;
            }

        }
    }
}
