#include "BufferSource.hpp"

#include <Engine.Common/Exception/NotImplementedException.hpp>

using namespace hg::engine::resource;
using namespace hg;

BufferSource::BufferSource(mref<Buffer> buffer_) :
    _buffer(_STD move(buffer_)) {}

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

streamsize BufferSource::size() const {
    return _buffer.size;
}

streamsize BufferSource::estimatedSize() const noexcept {
    return _buffer.size;
}

bool BufferSource::get(streamoff offset_, streamsize size_, ptr<void> dst_, ref<streamsize> actualSize_) {

    if (!isReady() || offset_ >= _buffer.size) {
        return false;
    }

    actualSize_ = MIN(_buffer.size - offset_, size_);
    memcpy(dst_, static_cast<ptr<u8>>(_buffer.mem) + offset_, actualSize_);

    return true;
}

hg::concurrent::future<Source::async_result_value> BufferSource::get(streamoff offset_, streamsize size_) {
    throw NotImplementedException {};
}

bool BufferSource::write(streamoff offset_, streamsize size_, const ptr<void> src_, ref<streamsize> actualSize_) {

    if (not isReady() || offset_ >= _buffer.size) {
        return false;
    }

    actualSize_ = MIN(_buffer.size - offset_, size_);
    memcpy(static_cast<ptr<u8>>(_buffer.mem) + offset_, src_, actualSize_);

    return true;
}

hg::concurrent::future<Source::async_write_result> BufferSource::write(
    streamoff offset_,
    streamsize size_,
    const ptr<void> src_
) {
    throw NotImplementedException {};
}
