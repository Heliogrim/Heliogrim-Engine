#include "NetworkSource.hpp"

#include <Engine.Common/Exception/NotImplementedException.hpp>

using namespace ember::engine::resource;
using namespace ember;

NetworkSource::reference_type NetworkSource::operator=(mref<value_type> other_) noexcept {
    if (this != _STD addressof(other_)) {
        throw NotImplementedException {};
    }

    return *this;
}

bool NetworkSource::isAsync() const noexcept {
    return false;
}

bool NetworkSource::isReady() const noexcept {
    throw NotImplementedException {};
}

bool NetworkSource::isReadable() const noexcept {
    throw NotImplementedException {};
}

bool NetworkSource::isWritable() const noexcept {
    throw NotImplementedException {};
}

streamsize NetworkSource::size() const {
    return -1;
}

streamsize NetworkSource::estimatedSize() const noexcept {
    return -1;
}

bool NetworkSource::get(streamoff offset_, streamsize size_, ptr<void> dst_, ref<streamsize> actualSize_) {
    throw NotImplementedException {};
}

ember::concurrent::future<Source::async_result_value> NetworkSource::get(streamoff offset_, streamsize size_) {
    throw NotImplementedException {};
}

bool NetworkSource::write(streamoff offset_, streamsize size_, const ptr<void> src_, ref<streamsize> actualSize_) {
    throw NotImplementedException {};
}

ember::concurrent::future<Source::async_write_result> NetworkSource::write(
    streamoff offset_,
    streamsize size_,
    const ptr<void> src_
) {
    throw NotImplementedException {};
}
