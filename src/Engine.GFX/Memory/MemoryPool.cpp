#include "MemoryPool.hpp"

#ifdef _DEBUG
#include <cassert>
#endif

#include "AllocatedMemory.hpp"
#include "AllocationResult.hpp"

using namespace ember::engine::gfx::memory;
using namespace ember;

MemoryPool::MemoryPool(cref<MemoryLayout> layout_) :
    _layout(layout_) {}

MemoryPool::~MemoryPool() {
    tidy();
}

void MemoryPool::tidy() {
    for (const auto* entry : _memory) {
        #ifdef _DEBUG
        assert(!entry->parent);
        #endif
    }

    _memory.clear();
    _memory.shrink_to_fit();
}

cref<MemoryLayout> MemoryPool::layout() const noexcept {
    return _layout;
}

AllocationResult MemoryPool::allocate(const u64 size_, ref<ptr<AllocatedMemory>> dst_) {
    // TODO: Implement
    return AllocationResult::eOutOfMemory;
}

bool MemoryPool::free(mref<ptr<AllocatedMemory>> mem_) {
    // TODO: Implement
    return false;
}
