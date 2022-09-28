#include "BufferSource.hpp"

#include <Engine.Common/Exception/NotImplementedException.hpp>

using namespace ember::engine::res;
using namespace ember;

BufferSource::reference_type BufferSource::operator=(mref<value_type> other_) noexcept {
    if (this != _STD addressof(other_)) {
        _buffer = _STD move(other_._buffer);
    }

    return *this;
}

bool BufferSource::isAsync() const noexcept {
    return false;
}

bool BufferSource::isReady() const noexcept {
    return _buffer.mem;
}

bool BufferSource::isReadable() const noexcept {
    return true;
}

bool BufferSource::isWritable() const noexcept {
    return true;
}

bool BufferSource::get(u64 offset_, u64 size_, ptr<void> dst_, ref<u64> actualSize_) {
    if (!isReady() || offset_ >= _buffer.size) {
        return false;
    }

    actualSize_ = MIN(_buffer.size - offset_, size_);
    memcpy(dst_, _buffer.mem, actualSize_);

    return true;
}

concurrent::future<Source::async_result_value> BufferSource::get(u64 offset_, u64 size_) {
    throw NotImplementedException {};
}
