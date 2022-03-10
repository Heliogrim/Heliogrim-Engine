#include "AllocatedMemory.hpp"

using namespace ember::engine::gfx;
using namespace ember;

AllocatedMemory::~AllocatedMemory() {
    if (mapping) {
        unmap();
    }

    if (vkMemory) {
        vkDevice.freeMemory(vkMemory);
    }
}

ref<AllocatedMemory> AllocatedMemory::operator=(mref<AllocatedMemory> other_) noexcept {

    if (_STD addressof(other_) != this) {
        _STD swap(props, other_.props);
        _STD swap(align, other_.align);
        _STD swap(size, other_.size);
        _STD swap(mapping, other_.mapping);
        _STD swap(vkMemory, other_.vkMemory);
        _STD swap(vkDevice, other_.vkDevice);
    }

    return *this;
}

void AllocatedMemory::flush(const u64 size_, const u64 offset_) {
    const vk::MappedMemoryRange range { vkMemory, offset_, size_ };
    [[maybe_unused]] auto result = vkDevice.flushMappedMemoryRanges(1, &range);
}

MemoryMapping AllocatedMemory::map(const u64 size_, const u64 offset_) {
    mapping = vkDevice.mapMemory(vkMemory, offset_, size_, vk::MemoryMapFlags {});
    return mapping;
}

void AllocatedMemory::unmap() {
    assert(mapping != nullptr);

    vkDevice.unmapMemory(vkMemory);
    mapping = nullptr;
}

bool AllocatedMemory::write(const ptr<void> data_, const u64 size_) {
    if (!mapping) {
        return false;
    }

    memcpy(mapping, data_, static_cast<size_t>(size_));
    return true;
}
