#include "ParallelSource.hpp"

#include <algorithm>
#include <ranges>
#include <Engine.Common/Exception/NotImplementedException.hpp>

using namespace ember::engine::res;
using namespace ember;

ParallelSource::reference_type ParallelSource::operator=(mref<value_type> other_) noexcept {
    if (this != _STD addressof(other_)) {
        _sources = _STD move(other_._sources);
    }

    return *this;
}

bool ParallelSource::isAsync() const noexcept {
    return _STD ranges::any_of(_sources, [](const auto& entry) {
        return entry->isAsync();
    });
}

bool ParallelSource::isReady() const noexcept {
    return _STD ranges::any_of(_sources, [](const auto& entry) {
        return entry->isReady();
    });
}

bool ParallelSource::isReadable() const noexcept {
    return _STD ranges::any_of(_sources, [](const auto& entry) {
        return entry->isReadable();
    });
}

bool ParallelSource::isWritable() const noexcept {
    return _STD ranges::any_of(_sources, [](const auto& entry) {
        return entry->isWritable();
    });
}

bool ParallelSource::get(streamoff offset_, streamsize size_, ptr<void> dst_, ref<streamsize> actualSize_) {
    throw NotImplementedException {};
}

ember::concurrent::future<Source::async_result_value> ParallelSource::get(streamoff offset_, streamsize size_) {
    throw NotImplementedException {};
}

bool ParallelSource::write(streamoff offset_, streamsize size_, const ptr<void> src_, ref<streamsize> actualSize_) {
    throw NotImplementedException {};
}

ember::concurrent::future<Source::async_write_result> ParallelSource::write(
    streamoff offset_,
    streamsize size_,
    const ptr<void> src_
) {
    throw NotImplementedException {};
}
