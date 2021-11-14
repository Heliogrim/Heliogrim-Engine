#include "StagedQueue.hpp"

#ifdef _DEBUG
#include <cassert>
#endif

using namespace ember::engine::scheduler;
using namespace ember;

StagedQueue::StagedQueue(const ptr<SharedBufferPool> pool_) :
    _pool(pool_),
    _buffers(nullptr),
    _bufferCount(0),
    _bufferCapacity(0) {}

StagedQueue::~StagedQueue() {
    tidy();
}

void StagedQueue::tidy() {

    if (_bufferCapacity <= 0 && _buffers == nullptr) {
        return;
    }

    auto* const buffers = _buffers.load(_STD memory_order_relaxed);
    for (auto i = _bufferCount.load(_STD memory_order_relaxed); i > 0; --i) {
        _pool->release(_STD move(buffers[i - 1]));
    }

    delete[] _buffers;
    _bufferCapacity = 0;
}

void StagedQueue::growCapacity() {

    /**
     * Acquire
     */
    while (_blocked.test_and_set(_STD memory_order_acq_rel)) {
        ;// Spinning
    }

    /**
     * Acquire new resources
     */
    auto newCapacity = _bufferCapacity + 1;
    auto* newBuffers { static_cast<ptr<ptr<aligned_buffer>>>(malloc(sizeof(ptr<aligned_buffer>) * newCapacity)) };

    /**
     * Translocate old pointer to new memory block
     */
    const auto used = _bufferCount.load(_STD memory_order_consume);
    auto* oldBuffers = _buffers.load(_STD memory_order_consume);

    for (auto i = 0; i < used; ++i) {
        newBuffers[i] = oldBuffers[i];
    }

    /**
     * Swap used memory
     */
    _buffers.store(newBuffers, _STD memory_order_release);
    _bufferCapacity = newCapacity;

    /**
     * Free old resources
     */
    free(oldBuffers);

    /**
     * Release
     */
    _blocked.clear(_STD memory_order_release);
}

void StagedQueue::grow(const u8 lastUsage_) {

    /**
     * Acquire
     */
    while (_blocked.test_and_set(_STD memory_order_acq_rel)) {
        ;// Spinning
    }

    /**
     * Check for any left capacity
     */
    const auto used = _bufferCount.load(_STD memory_order_consume);

    /**
     * Check whether somebody else might changed usage.
     */
    if (used > lastUsage_) {
        return;
    }

    if (used < _bufferCapacity) {

        auto* const buffers = _buffers.load(_STD memory_order_consume);
        buffers[used] = _pool->acquire();

        --_bufferCount;

        /**
         * Release
         */
        _blocked.clear(_STD memory_order_release);

        return;
    }

    /**
     * Release
     */
    _blocked.clear(_STD memory_order_release);

    growCapacity();
    grow(lastUsage_);
}

void StagedQueue::push(mref<task::__TaskDelegate> task_) {

    /**
     * Try to store task into available queues
     */
    for (auto i = _bufferCount.load(_STD memory_order_consume); i > 0; --i) {

        auto* const buffer = _buffers.load(_STD memory_order_consume)[i];

        /**
         * Try to use current queue
         */
        if (!buffer->full()) {
            continue;
        }

        /**
         * TODO: Might check for task masking
         */
        if (buffer->try_push(_STD move(task_))) {
            return;
        }
    }

    /**
     * Grow and Retry
     */
    grow(_bufferCount.load(_STD memory_order_consume));

    // Warning: non-guaranteed callstack recursion
    push(_STD move(task_));
}

bool StagedQueue::pop(ref<ptr<aligned_buffer>> buffer_) {

    /**
     * Check whether any queue is available
     */
    const auto left = _bufferCount.load(_STD memory_order_consume);
    if (left <= 0) {
        return false;
    }

    /**
     * Acquire
     */
    while (_blocked.test_and_set(_STD memory_order_acq_rel)) {
        ;// Spinning
    }

    /**
     * Acquire ownership of lifo queue
     */
    const ptr<aligned_buffer> cpy = _buffers[left - 1];
    _buffers[left - 1] = nullptr;

    /**
     * Erase accessing index
     */
    auto expect { left };
    #ifdef _DEBUG
    DEBUG_ASSERT(
        _bufferCount.compare_exchange_strong(expect, left - 1, _STD memory_order_release, _STD memory_order_relaxed),
        "Failed to rewrite usage. Might be a race condition...")
    #else
    --_bufferCount;
    #endif

    /**
     * Release
     */
    _blocked.clear(_STD memory_order_release);

    /**
     * Move pointer
     */
    buffer_ = cpy;
    return true;
}
