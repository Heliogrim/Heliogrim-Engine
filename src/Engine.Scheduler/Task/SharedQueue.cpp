#include "SharedQueue.hpp"

using namespace hg::engine::scheduler::task;
using namespace hg;

constexpr u8 retry_threshold = 16u;

SharedSubQueue::SharedSubQueue(TaskMask mask_) noexcept :
    mask(mask_) { }

bool SharedSubQueue::try_acquire() {
    const auto tid = thread::self::getId();

    thread::thread_id expect { 0 };
    return owner.compare_exchange_weak(expect, tid, std::memory_order::relaxed, std::memory_order::relaxed);
}

void SharedSubQueue::acquire() {
    const auto tid = thread::self::getId();

    u8 tries { 0 };
    thread::thread_id expect { 0 };
    while (!owner.compare_exchange_weak(expect, tid, std::memory_order::release, std::memory_order::relaxed)) {
        expect = 0;

        if (++tries > retry_threshold) {
            thread::self::yield();
        }
    }
}

void SharedSubQueue::release() {
    #if _DEBUG
    auto expect { thread::self::getId() };
    const auto result = owner.compare_exchange_strong(
        expect,
        thread::thread_id { 0 },
        std::memory_order::release,
        std::memory_order::acquire
    );
    assert(result);
    #else
    owner.store(thread::thread_id { 0 }, std::memory_order::relaxed);
    #endif
}

SharedQueue::SharedQueue():
    _queues(nullptr),
    _queueCount(0) { }

SharedQueue::~SharedQueue() {
    if (_queueCount > 0 && _queues != nullptr) {
        destroy();
    }
}

void SharedQueue::setup(u32 threads_) {
    constexpr u8 count = sizeof(TaskMaskValues) / sizeof(TaskMask);
    constexpr u64 memsize = count * sizeof(aligned_subqueue);

    _queues = static_cast<aligned_subqueue*>(malloc(memsize));

    for (u8 i = 0; i < count; ++i) {
        ::new(&_queues[i]) aligned_subqueue {
            TaskMaskValues[i]
        };
    }

    _queueCount = count;
}

void SharedQueue::destroy() {
    for (u8 i = 0; i < _queueCount; ++i) {
        auto& cur = _queues[i];
        cur.acquire();
        cur.~SharedSubQueue();
    }

    free(_queues);

    _queues = nullptr;
    _queueCount = 0;
}

void SharedQueue::push(mref<non_owning_rptr<const task::TaskDelegate>> task_) {
    bool done = false;

    while (!done) {
        for (u8 i = 0; i < _queueCount; ++i) {
            auto& cur = _queues[i];

            if (cur.mask & task_->mask()) {

                cur.acquire();
                done = cur.buffer.try_push(std::forward<non_owning_rptr<const task::TaskDelegate>>(task_));
                cur.release();

                if (done) {
                    return;
                }
            }
        }
    }
}

void SharedQueue::pop(TaskMask mask_, ref<non_owning_rptr<const task::TaskDelegate>> result_) noexcept {

    for (u8 i = 0; i < _queueCount; ++i) {
        auto& cur = _queues[i];

        if (!(cur.mask & mask_))
            continue;

        if (cur.try_acquire()) {

            if (cur.buffer.try_pop(result_)) {
                cur.release();
                return;
            }

            cur.release();
        }
    }

    result_ = nullptr;
}
