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

bool ParallelSource::get(u64 offset_, u64 size_, ptr<void> dst_, ref<u64> actualSize_) {
    throw NotImplementedException {};
}

concurrent::future<Source::async_result_value> ParallelSource::get(u64 offset_, u64 size_) {
    throw NotImplementedException {};
}
