#pragma once

#include <Engine.Common/Concurrent/Collection/RingBuffer.hpp>

#include "../Task/Task.hpp"
#include "../Thread/Thread.hpp"

namespace ember::engine::scheduler {

    class SharedQueue {
    public:
        SharedQueue();

        ~SharedQueue();

    private:
        const task::TaskMask _mask;

    public:
        [[nodiscard]] task::TaskMask mask() const noexcept;

    private:
        _STD atomic<thread::thread_id> _owner;

    public:
        [[nodiscard]] bool try_acquire() noexcept;

        void acquire();

        void release();

    private:
        concurrent::RingBuffer<task::__TaskDelegate> _buffer;

    public:
        [[nodiscard]] ref<concurrent::RingBuffer<task::__TaskDelegate>> buffer() noexcept;
    };
}
