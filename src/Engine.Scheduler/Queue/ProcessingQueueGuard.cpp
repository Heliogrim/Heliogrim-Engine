#include "ProcessingQueueGuard.hpp"

#ifdef _DEBUG
#include <cassert>
#endif

using namespace ember::engine::scheduler;
using namespace ember;

ProcessingQueueGuard::ProcessingQueueGuard(const ptr<SharedBufferPool> pool_) noexcept :
    _pool(pool_),
    _buffer(nullptr) {}

ProcessingQueueGuard::~ProcessingQueueGuard() {
    #ifdef _DEBUG
    if (_buffer != nullptr) {
        DEBUG_ASSERT(_buffer->empty(), "Releasing task buffer which is not empty.")
    }
    #endif

    if (_buffer != nullptr) {
        _pool->release(_STD move(_buffer));
        _buffer = nullptr;
    }
}

void ProcessingQueueGuard::acquire() {

    if (_buffer != nullptr) {
        return;
    }

    /**
     *
     */
    _buffer = _pool->acquire();
}

void ProcessingQueueGuard::release() {

    if (!_buffer->empty()) {
        return;
    }

    /**
     *
     */
    _pool->release(_STD move(_buffer));
    _buffer = nullptr;
}

bool ProcessingQueueGuard::try_store(mref<ptr<aligned_buffer>> buffer_) noexcept {
    /**
     * Try to acquire producer lock
     */
    if (_pLck.test_and_set(_STD memory_order_release)) {
        return false;
    }

    /**
     * Try to acquire consumer lock
     */
    if (_cLck.test_and_set(_STD memory_order_release)) {
        _pLck.clear(_STD memory_order_release);
        return false;
    }

    /**
     * Check for old state
     */
    bool result { false };
    if (_buffer == nullptr || _buffer->empty()) {

        /**
         * Release if previous buffer exists, but is empty
         */
        if (_buffer != nullptr) {
            _pool->release(_STD move(_buffer));
        }

        /**
         * Store new buffer
         */
        _buffer = _STD move(buffer_);

        /**
         * Write feedback
         */
        result = true;
    }

    /**
     * Release
     */
    _pLck.clear(_STD memory_order_release);
    _cLck.clear(_STD memory_order_release);

    /**
     * Return feedback
     */
    return result;
}

bool ProcessingQueueGuard::store(mref<ptr<aligned_buffer>> buffer_) noexcept {

    /**
     * Warning: Might break, cause not atomic
     */
    if (buffer_ != nullptr) {
        return false;
    }

    /**
     * Try to acquire producer lock
     */
    while (_pLck.test_and_set(_STD memory_order_release)) {
        ;// Spinning
    }

    /**
     * Try to acquire consumer lock
     */
    while (_cLck.test_and_set(_STD memory_order_release)) {
        ;// Spinning
    }

    /**
     * Check for old state
     */
    bool result { false };
    if (_buffer == nullptr || _buffer->empty()) {

        /**
         * Release if previous buffer exists, but is empty
         */
        if (_buffer != nullptr) {
            _pool->release(_STD move(_buffer));
        }

        /**
         * Store new buffer
         */
        _buffer = _STD move(buffer_);

        /**
         * Write feedback
         */
        result = true;
    }

    /**
     * Release
     */
    _pLck.clear(_STD memory_order_release);
    _cLck.clear(_STD memory_order_release);

    /**
     * Return feedback
     */
    return result;
}

bool ProcessingQueueGuard::try_push(mref<task::__TaskDelegate> task_) noexcept {

    /**
     * Acquire producer lock
     */
    if (_pLck.test_and_set(_STD memory_order_release)) {
        return false;
    }

    /**
     *
     */
    bool result { false };
    if (_buffer != nullptr) {
        /**
         * Try to push a task to the stored buffer
         */
        result = _buffer->try_push(_STD move(task_));

    } else {
        /**
         * Try acquire consumer lock
         */
        if (!_cLck.test_and_set(_STD memory_order_release)) {
            acquire();
            _cLck.clear(_STD memory_order_release);
        }

        if (_buffer != nullptr) {
            /**
             * Try to push a task to the stored buffer
             */
            result = _buffer->try_push(_STD move(task_));
        }
    }

    /**
     * Release producer lock
     */
    _pLck.clear(_STD memory_order_release);

    /**
     *
     */
    return result;
}

bool ProcessingQueueGuard::try_pop(const task::TaskMask mask_, ref<task::__TaskDelegate> task_) noexcept {

    /**
     * Acquire consumer lock
     */
    if (_cLck.test_and_set(_STD memory_order_release)) {
        return false;
    }

    /**
     *
     */
    bool result { false };
    if (_buffer != nullptr) {

        /**
         * Try to pop a task from the stored buffer
         */
        result = _buffer->try_pop(task_);
    }

    /**
     * Check if we got casually empty
     */
    if (_buffer != nullptr && _buffer->empty()) {
        /**
         * Try acquire producer lock
         */
        if (!_pLck.test_and_set(_STD memory_order_release)) {
            release();
            _pLck.clear(_STD memory_order_release);
        }
    }

    /**
     * Release consumer lock
     */
    _cLck.clear(_STD memory_order_release);
    return result;
}
