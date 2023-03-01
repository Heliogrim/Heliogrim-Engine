#include "PooledBuffer.hpp"

#include "SharedBufferPool.hpp"

using namespace hg::engine::scheduler;
using namespace hg;

PooledBuffer::PooledBuffer(const ptr<SharedBufferPool> pool_) noexcept :
    RingBuffer(),
    _pool(pool_) {}

PooledBuffer::PooledBuffer(const ptr<SharedBufferPool> pool_, const underlying_type::size_type reserved_) :
    RingBuffer(reserved_),
    _pool(pool_) {}

PooledBuffer::~PooledBuffer() = default;

void PooledBuffer::release() {
    /**
     *
     */
    _pool->release(_STD move(this));
}

bool PooledBuffer::try_pop(ref<non_owning_rptr<const task::TaskDelegate>> value_) {
    /**
     *
     */
    if (_reader.test_and_set(_STD memory_order_release)) {
        return false;
    }

    auto result = underlying_type::try_pop(value_);

    /**
     *
     */
    _reader.clear(_STD memory_order_release);
    return result;
}

bool PooledBuffer::try_push(mref<non_owning_rptr<const task::TaskDelegate>> value_) {
    /**
     *
     */
    if (_writer.test_and_set(_STD memory_order_release)) {
        return false;
    }

    auto result = underlying_type::try_push(_STD move(value_));

    /**
     *
     */
    _writer.clear(_STD memory_order_release);
    return result;
}

bool PooledBuffer::empty() const noexcept {
    return underlying_type::empty();
}

bool PooledBuffer::full() const noexcept {
    return underlying_type::full();
}
