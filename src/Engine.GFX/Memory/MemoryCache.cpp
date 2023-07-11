#include "MemoryCache.hpp"

#include <Engine.Common/Make.hpp>

#include "AllocatedMemory.hpp"
#include "AllocationResult.hpp"
#include "MemoryPool.hpp"

using namespace hg::engine::gfx::memory;
using namespace hg;

MemoryCache::MemoryCache() noexcept :
    _pools() {}

MemoryCache::~MemoryCache() {
    tidy();
}

void MemoryCache::tidy() {
    for (const auto& entry : _pools) {
        entry.second->tidy();
        delete entry.second;
    }
    _pools.clear();
}

non_owning_rptr<MemoryPool> MemoryCache::getOrCreatePool(cref<MemoryLayout> layout_) {

    const auto it { _pools.find(layout_) };
    if (it != _pools.end()) {
        return it->second;;
    }

    auto* pool { make_ptr<MemoryPool>(layout_) };
    const auto insert { _pools.insert_or_assign(layout_, pool) };

    assert(insert.second);

    return pool;
}

AllocationResult MemoryCache::allocate(cref<MemoryLayout> layout_, const u64 size_, ref<ptr<AllocatedMemory>> dst_) {
    const auto it { _pools.find(layout_) };
    if (it == _pools.end()) {
        return AllocationResult::eFailed;
    }

    return it->second->allocate(size_, dst_);
}

bool MemoryCache::free(mref<ptr<AllocatedMemory>> mem_) {

    const auto layout { mem_->layout };
    const auto it { _pools.find(layout) };

    if (it == _pools.end()) {
        return false;
    }

    return it->second->free(_STD move(mem_));
}
