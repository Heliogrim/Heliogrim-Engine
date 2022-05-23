#include "GlobalPooledAllocator.hpp"

#include "AllocationResult.hpp"
#include "Allocator.hpp"

using namespace ember::engine::gfx::memory;
using namespace ember;

GlobalPooledAllocator::GlobalPooledAllocator(cref<sptr<Allocator>> nativeAllocator_) :
    _alloc(nativeAllocator_) {}

GlobalPooledAllocator::~GlobalPooledAllocator() {
    tidy();
}

void GlobalPooledAllocator::tidy() {
    _cache.tidy();
}

AllocationResult GlobalPooledAllocator::allocate(cref<MemoryLayout> layout_, const u64 size_,
    ref<ptr<AllocatedMemory>> dst_) {

    if (_cache.allocate(layout_, size_, dst_) == AllocationResult::eSuccess) {
        return AllocationResult::eSuccess;
    }

    return _alloc->allocate(layout_, size_, dst_);
}

void GlobalPooledAllocator::free(mref<ptr<AllocatedMemory>> mem_) {

    if (_cache.free(_STD move(mem_))) {
        return;
    }

    _alloc->free(_STD move(mem_));
}

#include "../Device/Device.hpp"
#include "../API/VkTranslate.hpp"

AllocationResult engine::gfx::memory::allocate(
    const ptr<GlobalPooledAllocator> alloc_,
    cref<sptr<Device>> device_,
    cref<vk::Buffer> buffer_,
    cref<MemoryProperties> props_,
    ref<ptr<AllocatedMemory>> dst_
) {

    const auto req { device_->vkDevice().getBufferMemoryRequirements(buffer_) };
    const MemoryLayout layout {
        req.alignment,
        props_,
        req.memoryTypeBits
    };

    return alloc_->allocate(layout, req.size, dst_);
}

AllocationResult engine::gfx::memory::allocate(
    const ptr<GlobalPooledAllocator> alloc_,
    cref<sptr<Device>> device_,
    vk::Image image_,
    cref<MemoryProperties> props_,
    ref<ptr<AllocatedMemory>> dst_
) {

    const auto req { device_->vkDevice().getImageMemoryRequirements(image_) };
    const MemoryLayout layout {
        req.alignment,
        props_,
        req.memoryTypeBits
    };

    return alloc_->allocate(layout, req.size, dst_);
}
