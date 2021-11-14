#pragma once

#include "SharedBufferPool.hpp"

namespace ember::engine::scheduler {

    class StagedQueue {
    public:
        using aligned_buffer = SharedBufferPool::aligned_buffer;

    public:
        StagedQueue(_In_ const ptr<SharedBufferPool> pool_);

        ~StagedQueue();

    private:
        void tidy();

    private:
        ptr<SharedBufferPool> _pool;

    private:
        _STD atomic<ptr<ptr<aligned_buffer>>> _buffers;
        _STD atomic_uint_fast8_t _bufferCount;
        uint_fast8_t _bufferCapacity;

    private:
        _STD atomic_flag _blocked;

    private:
        void growCapacity();

        void grow(_In_ const u8 lastUsage_);

    public:
        void push(_Inout_ mref<task::__TaskDelegate> task_);

        _Success_(return == true) bool pop(_Inout_ ref<ptr<aligned_buffer>> buffer_);
    };
}
