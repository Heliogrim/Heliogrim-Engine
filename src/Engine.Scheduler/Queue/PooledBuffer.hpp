#pragma once
#include <Engine.Common/Concurrent/Collection/RingBuffer.hpp>

#include "../Task/Task.hpp"

namespace ember::engine::scheduler {

    /**
     * Forward Declaration
     */
    class SharedBufferPool;

    class PooledBuffer :
        protected concurrent::RingBuffer<task::__TaskDelegate> {
    public:
        using underlying_type = concurrent::RingBuffer<task::__TaskDelegate>;

    public:
        PooledBuffer(_In_ const ptr<SharedBufferPool> pool_) noexcept;

        PooledBuffer(_In_ const ptr<SharedBufferPool> pool_, _In_ const underlying_type::size_type reserved_);

        ~PooledBuffer();

    private:
        ptr<SharedBufferPool> _pool;

    public:
        /**
         * Release this buffer back to pool
         *
         * @author Julius
         * @date 17.11.2021
         */
        void release();

    private:
        _STD atomic_flag _writer;
        _STD atomic_flag _reader;

    public:
        [[nodiscard]] bool try_pop(_Out_ ref<task::__TaskDelegate> value_);

        [[nodiscard]] bool try_push(_Inout_ mref<task::__TaskDelegate> value_);

        [[nodiscard]] bool empty() const noexcept;

        [[nodiscard]] bool full() const noexcept;
    };
}
