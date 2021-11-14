#include "SharedQueue.hpp"

#ifdef _DEBUG
#include <assert.h>
#endif

using namespace ember::engine::scheduler;
using namespace ember;

constexpr u8 retry_threshold = 16ui8;

SharedQueue::SharedQueue() :
    _mask(),
    _owner(),
    _buffer() {}

SharedQueue::~SharedQueue() {
    #ifdef _DEBUG
    assert(_owner.load() == 0);
    #endif
}

task::TaskMask SharedQueue::mask() const noexcept {
    return _mask;
}

bool SharedQueue::try_acquire() noexcept {

    const auto threadId { thread::self::getId() };

    thread::thread_id expect { 0 };
    return _owner.compare_exchange_weak(expect, threadId, _STD memory_order_relaxed, _STD memory_order_relaxed);
}

void SharedQueue::acquire() {

    const auto threadId { thread::self::getId() };

    u8 retries { 0 };
    thread::thread_id expect { 0 };
    while (_owner.compare_exchange_weak(expect, threadId, _STD memory_order_release, _STD memory_order_relaxed)) {
        /**
         * Reset expected
         */
        expect = 0;

        /**
         * Yield if do much cycling
         */
        if (++retries > retry_threshold) {
            thread::self::yield();
            // retries -= retry_threshold;
        }
    }

}

void SharedQueue::release() {
    #ifdef _DEBUG

    auto expect { thread::self::getId() };
    assert(
        _owner.compare_exchange_strong(expect, thread::thread_id{0}, _STD memory_order_release, _STD
            memory_order_relaxed));

    #else
    _owner.store(thread::thread_id {0}, _STD memory_order_relaxed);
    #endif
}

ref<engine::concurrent::RingBuffer<const task::TaskDelegate*>> SharedQueue::buffer() noexcept {
    return _buffer;
}
