#include "NetworkSource.hpp"

#include <Engine.Common/Exception/NotImplementedException.hpp>

using namespace ember::engine::res;
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

bool NetworkSource::get(u64 offset_, u64 size_, ptr<void> dst_, ref<u64> actualSize_) {
    throw NotImplementedException {};
}

concurrent::future<Source::async_result_value> NetworkSource::get(u64 offset_, u64 size_) {
    throw NotImplementedException {};
}
