#include "AllocatedMemory.hpp"

#ifdef _DEBUG
#include <cassert>
#endif

#include "Allocator.hpp"

using namespace ember::engine::gfx::memory;
using namespace ember;

AllocatedMemory::~AllocatedMemory() {
    if (mapping) {
        unmap();
    }

    #ifdef _DEBUG
    if (parent || allocator) {
        assert(!vkMemory && "Possible memory leak due to missing hierarchical release report.");
    }
    #else
    if (vkMemory && (parent || allocator)) {
        throw _STD runtime_error("Possible memory leak due to missing hierarchical release report.");
    }
    #endif

    if (vkMemory && !parent && !allocator) {
        vkDevice.freeMemory(vkMemory);
    }
}

ref<AllocatedMemory> AllocatedMemory::operator=(mref<AllocatedMemory> other_) noexcept {

    if (_STD addressof(other_) != this) {
        _STD swap(allocator, other_.allocator);
        _STD swap(parent, other_.parent);
        _STD swap(layout, other_.layout);
        _STD swap(size, other_.size);
        _STD swap(mapping, other_.mapping);
        _STD swap(vkMemory, other_.vkMemory);
        _STD swap(vkDevice, other_.vkDevice);
    }

    return *this;
}

void AllocatedMemory::flush(const u64 size_, const u64 offset_) {

    const auto mapSize { (size_ == VK_WHOLE_SIZE && offset_ == 0 && offset != 0) ? size : size_ };

    #ifdef _DEBUG
    assert((mapSize == VK_WHOLE_SIZE && (offset_ + offset) == 0) || mapSize <= (size - offset_));
    #endif

    const vk::MappedMemoryRange range { vkMemory, offset + offset_, mapSize };
    [[maybe_unused]] auto result = vkDevice.flushMappedMemoryRanges(1, &range);
}

MemoryMapping AllocatedMemory::map(const u64 size_, const u64 offset_) {

    const auto mapSize { (size_ == VK_WHOLE_SIZE && offset_ == 0 && offset != 0) ? size : size_ };

    #ifdef _DEBUG
    assert((mapSize == VK_WHOLE_SIZE && (offset_ + offset) == 0) || mapSize <= (size - offset_));
    #endif

    mapping = vkDevice.mapMemory(vkMemory, offset + offset_, mapSize, vk::MemoryMapFlags {});
    return mapping;
}

void AllocatedMemory::unmap() {
    assert(mapping != nullptr);

    vkDevice.unmapMemory(vkMemory);
    mapping = nullptr;
}

bool AllocatedMemory::write(const ptr<const void> data_, const u64 size_) {
    if (!mapping) {
        return false;
    }

    memcpy(mapping, data_, static_cast<size_t>(size_));
    return true;
}

void AllocatedMemory::free(mref<ptr<AllocatedMemory>> memory_) {

    if (memory_->mapping) {
        memory_->unmap();
    }

    if (memory_->allocator) {
        return memory_->allocator->free(_STD move(memory_));
    }

    return delete _STD move(memory_);
}
