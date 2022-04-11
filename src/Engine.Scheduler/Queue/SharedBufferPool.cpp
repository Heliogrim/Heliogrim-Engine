#include "SharedBufferPool.hpp"

#ifdef _DEBUG
#include <cassert>
#endif

using namespace ember::engine::scheduler;
using namespace ember;

SharedBufferPool::SharedBufferPool() :
    _pooled(nullptr),
    _poolSize(0),
    _poolCapacity(0),
    _blocked() {}

SharedBufferPool::~SharedBufferPool() {
    tidy();
}

void SharedBufferPool::tidy() {

    if (_poolCapacity <= 0) {
        return;
    }

    /*
    // TODO: Should be irrelated to race-condition, but seams to use corrupted memory or ...
    while (_blocked.test_and_set()) {
        ;// Spinning
    }
     */

    const size_type size = _poolSize.load(_STD memory_order_relaxed);
    auto* pooled = _pooled.load(_STD memory_order_relaxed);

    for (size_type i = 0; i < size; ++i) {
        delete pooled[i];
    }

    free(pooled);
    _poolCapacity = 0;
}

cref<SharedBufferPool::atomic_size_type> SharedBufferPool::size() const noexcept {
    return _poolSize;
}

SharedBufferPool::capacity_type SharedBufferPool::capacity() const noexcept {
    return _poolCapacity;
}

ptr<SharedBufferPool::aligned_buffer> SharedBufferPool::acquire() {

    /**
     * Precheck for available reusage
     */
    if (_poolSize.load(_STD memory_order_consume) <= 0) {
        return new aligned_buffer { this };
    }

    /**
     * Acquire
     */
    while (_blocked.test_and_set(_STD memory_order_acq_rel)) {
        ;// Spinning
    }

    /**
     * Recheck available
     */
    const auto size = _poolSize.load(_STD memory_order_consume);

    if (size <= 0) {
        /**
         * Release
         */
        _blocked.clear(_STD memory_order_release);
        return acquire();
    }

    /**
     * Reuse stored resource
     */
    auto* const pooled = _pooled.load(_STD memory_order_consume);
    auto* buffer = _STD move(pooled[size - 1]);

    // TODO:
    pooled[size - 1] = nullptr;
    --_poolSize;

    /**
     * Release
     */
    _blocked.clear(_STD memory_order_release);
    return buffer;
}

void SharedBufferPool::release(mref<ptr<aligned_buffer>> buffer_) {

    /**
     * Acquire
     */
    while (_blocked.test_and_set(_STD memory_order_acq_rel)) {
        ;// Spinning
    }

    /**
     * Check for any left capacity
     */
    const auto used = _poolSize.load(_STD memory_order_consume);

    if (used < _poolCapacity) {

        auto* const pooled = _pooled.load(_STD memory_order_consume);
        pooled[used] = _STD move(buffer_);

        ++_poolSize;

        #ifdef _DEBUG
        for (decltype(_poolSize)::value_type i = 0; i < used; ++i) {
            DEBUG_ASSERT(pooled[i] != buffer_, "Collision by double releasing buffer to pool.")
        }
        #endif

        /**
         * Release
         */
        _blocked.clear(_STD memory_order_release);

        return;
    }

    grow(_poolCapacity + 1ui16);

    /**
     * Release
     */
    _blocked.clear(_STD memory_order_release);

    release(_STD move(buffer_));
}

void SharedBufferPool::grow(const capacity_type capacity_) {

    /**
     * Allocate new resources
     */
    auto* newPooled { static_cast<ptr<ptr<aligned_buffer>>>(calloc(capacity_, sizeof(ptr<aligned_buffer>))) };

    /**
     * Translocate old pointers to new memory block
     */
    const auto size = _poolSize.load(_STD memory_order_consume);
    auto* oldPooled = _pooled.load(_STD memory_order_consume);

    for (size_type i = 0; i < size; ++i) {
        newPooled[i] = oldPooled[i];
    }

    /**
     * Swap
     */
    _pooled.store(newPooled, _STD memory_order_release);
    _poolCapacity = capacity_;

    /**
     * Free old resources
     */
    free(oldPooled);
}

bool SharedBufferPool::reserve(const size_type size_) {
    /**
     * Prepare at least required capacity
     */
    grow(MAX(size_, _poolCapacity));

    /**
     * Create requested elements
     */
    for (size_type ci = size_; ci > 0 && _poolSize.load(_STD memory_order_relaxed) < size_; --ci) {
        release(_STD move(new aligned_buffer { this }));
    }

    return true;
}
