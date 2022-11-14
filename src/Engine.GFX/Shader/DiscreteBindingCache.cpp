#include "DiscreteBindingCache.hpp"

#include "../Device/Device.hpp"
#include "DiscreteBindingPool.hpp"
#include "Engine.Common/Exception/NotImplementedException.hpp"

using namespace ember::engine::gfx;
using namespace ember;

DiscreteBindingCache::DiscreteBindingCache(cref<sptr<Device>> device_) noexcept :
    _device(device_) {}

DiscreteBindingCache::~DiscreteBindingCache() {
    tidy();
}

void DiscreteBindingCache::tidy() {
    for (const auto& entry : _mappedPools) {
        for (auto* pool : entry.second) {
            delete pool;
        }
    }

    _mappedPools.clear();
}

bool DiscreteBindingCache::prepare(cref<shader::ShaderBindingGroupLayout> layout_, const u32 sets_) {

    auto& entry { _mappedPools[layout_] };
    if (entry.empty()) {
        entry.push_back(new DiscreteBindingPool { _device, layout_, sets_ });
        return true;
    }

    const auto left { entry.front()->leftSets() };
    if (left < sets_) {
        entry.insert(entry.begin(),
            new DiscreteBindingPool { _device, layout_, entry.front()->maxSets() + (sets_ - left) });
        return true;
    }

    return false;
}

shader::DiscreteBindingGroup DiscreteBindingCache::allocate(cref<shader::ShaderBindingGroup> group_) {
    const shader::ShaderBindingGroupLayout& layout { group_.layout() };
    auto& entry { _mappedPools[layout] };

    if (entry.empty()) {
        entry.push_back(new DiscreteBindingPool { _device, layout, initial_pool_size });
    }

    auto* pool { entry.front() };
    if (pool->leftSets() <= 0ui32) {
        pool->markAsExceeded();
    }

    if (pool->exceeded()) {
        // [0] -> 15 | [1] -> 31 | [2] -> 63 | [4] -> 127 | [5] -> 255 | [6] -> 511 | [7] -> 1023 | [8] -> 2047
        entry.insert(entry.begin(), new DiscreteBindingPool { _device, layout, pool->maxSets() << 2ui64 });
        return allocate(group_);
    }

    return pool->allocate(group_);
}

ref<shader::DiscreteBindingGroup> DiscreteBindingCache::reallocate(ref<shader::DiscreteBindingGroup> group_) {
    throw NotImplementedException();
}

void DiscreteBindingCache::free(mref<shader::DiscreteBindingGroup> group_) {
    throw NotImplementedException();
}
