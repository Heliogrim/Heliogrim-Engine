#include "ProcessingQueue.hpp"

#include <Engine.Common/stdafx.h>

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
    _sharedCtrlBlockPage(),
    _currentPage(nullptr),
    _pageCtrl {},
    _resizing() {}

ProcessingQueue::~ProcessingQueue() {
    tidy();
}

void ProcessingQueue::tidy() {

    /**
     * Signal every ctrl block of shared task queue
     */
    for (auto i = 0; i < 16; ++i) {
        _sharedCtrlBlockPage.retire(i);
    }

    /**
     * Tidy dynamic pages
     */
    auto pagePtr = _pageCtrl[0].acq();
    if (!pagePtr.empty()) {
        pagePtr->tidy(true);

        if (_currentPage.load(_STD memory_order_relaxed) != &_pageCtrl[0]) {
            _pageCtrl[0].rel();
        }
    }

    pagePtr = _pageCtrl[1].acq();
    if (!pagePtr.empty()) {
        pagePtr->tidy(true);

        if (_currentPage.load(_STD memory_order_relaxed) != &_pageCtrl[1]) {
            _pageCtrl[1].rel();
        }
    }

    pagePtr.reset();
}

void ProcessingQueue::setup(const u64 workerCount_, const u64 maxSharedTasks_) {

    const auto sharedPageCount = maxSharedTasks_ / (31ui64 * 16ui64);
    DEBUG_ASSERT(sharedPageCount <= 1, "Currently unsuported amount of shared tasks.")

    /**
     * Setup shared task queues
     */
    for (auto i = 0; i < 16; ++i) {
        _sharedCtrlBlockPage.store(i, _pool->acquire());
    }

    /**
     * Prepare dynamic pages
     */
    constexpr u64 initialDynamicPages = 1;
    auto newPages = new DynamicBufferPages(initialDynamicPages, &_resizing);

    for (auto i = 0; i < initialDynamicPages; ++i) {
        newPages->page(i) = new DynamicBufferPages::page_value_type {};
    }

    _pageCtrl[0].push(newPages);
    _currentPage.store(&_pageCtrl[0], _STD memory_order_release);
}

void ProcessingQueue::grow() {

    /**
     * Check if resize is in progress
     */
    if (_resizing.test_and_set(_STD memory_order_release)) {
        return;
    }

    auto* currentCtrl = _currentPage.load(_STD memory_order_relaxed);
    /**
     * Get current pages and bump reference count
     *
     * @see AtomicCtrlBlockPage::retire(...)
     */
    auto currentPages = currentCtrl->acq();
    const auto currentPageCount = currentPages->size();

    /**
     * Make new dynamic pages
     */
    const auto nextPageCount = currentPageCount + 1;
    auto newPages = new DynamicBufferPages(nextPageCount, &_resizing);

    /**
     * Copy current state
     */
    for (u64 i = 0; i < currentPageCount; ++i) {
        newPages->page(i) = currentPages->page(i);
    }

    /**
     * Fill empty new state
     */
    for (auto i = currentPageCount; i < nextPageCount; ++i) {
        newPages->page(i) = new DynamicBufferPages::page_value_type {};
    }

    /**
     * Store new pages
     */
    const auto storeIdx = _pageCtrl/* &_pageCtrl[0] */ == currentCtrl ? 1 : 0;
    _pageCtrl[storeIdx].push(newPages);

    /**
     * Publish change
     */
    _currentPage.store(&_pageCtrl[storeIdx], _STD memory_order_release);

    /**
     * Retire previous state
     *
     * @see AtomicCtrlBlockPage::retire(...)
     */
    currentCtrl->rel();
}

void ProcessingQueue::push(mref<task::__TaskDelegate> task_) {

    #define inc(var_, limit_) ((++var_) >= limit_ ? var_ -= limit_ : var_)
    auto threadIdx = thread::self::getIdx();
    while (threadIdx > 16ui64) {
        threadIdx -= 16ui64;
    }

    /**
     * Loop shared storage from current thread index to current thread index by rountrip
     */
    u64 i = threadIdx;
    bool succeeded = false;
    for (i = inc(i, 16ui64); !succeeded && i != threadIdx; i = inc(i, 16ui64)) {
        /**
         * Get managed queue
         */
        auto managed = _sharedCtrlBlockPage.get(i);

        /**
         *
         */
        if (!managed.empty()) {
            succeeded = managed->try_push(_STD move(task_));
        }
    }

    #undef inc

    #if FALSE
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
    #endif

}

void ProcessingQueue::pushStaged(mref<ptr<aligned_buffer>> buffer_) {

    /**
     * First try pushing stage without waiting
     */
    while (true) {
        /**
         * Loop over every available queue, but enforce valid index range
         */
        auto* dynamicPagesCtrl = _currentPage.load(_STD memory_order_relaxed);
        const auto dynamicPages = dynamicPagesCtrl->acq();
        const auto dynamicPageCount = dynamicPages->size();

        for (size_type i = 0; i < dynamicPageCount; ++i) {

            /**
             * Get current dynamic page
             */
            const auto& page = dynamicPages->page(i);

            /**
             * Try to use current page
             */
            for (size_type j = 0; j < page_size; ++j) {
                /**
                 * Will already check if empty internally and faster than acquiring a managed pointer to check
                 */
                if (page->try_store(j, _STD move(buffer_))) {
                    return;
                }
            }
        }

        /**
         * Failed to push task to current available slots
         */
        // grow();
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
     * Speedup spread
     */
    auto threadIdx = thread::self::getIdx();
    while (threadIdx > 16ui64) {
        threadIdx -= 16ui64;
    }

    /**
     * Loop over every available queue, but enforce valid index range
     */
    auto* dynamicPagesCtrl = _currentPage.load(_STD memory_order_acquire);
    const auto dynamicPages = dynamicPagesCtrl->acq();
    const auto dynamicPageCount = dynamicPages->size();

    for (u64 i = 0; i < dynamicPageCount; ++i) {

        /**
         * Get current dynamic page
         */
        const auto& page = dynamicPages->page(i);

        /**
         * Try to use current page
         */
        for (u64 j = 0; j < page_size; ++j) {

            const auto bufferPtr = page->get(j);
            if (!bufferPtr.empty()) {

                if (bufferPtr->try_pop(task_)) {
                    return;
                }

                if (bufferPtr->empty()) {
                    page->retire(j);
                }
            }
        }
    }

    #define inc(var_, limit_) ((++var_) >= limit_ ? var_ -= limit_ : var_)

    /**
     * Loopup shared task queues
     */
    u64 i = threadIdx;
    bool succeeded = false;
    for (i = inc(i, 16ui64); !succeeded && i != threadIdx; i = inc(i, 16ui64)) {
        /**
         * Get managed queue
         */
        auto managed = _sharedCtrlBlockPage.get(i);

        /**
         *
         */
        if (!managed.empty()) {
            succeeded = managed->try_pop(task_);
        }
    }

    #undef inc
}
