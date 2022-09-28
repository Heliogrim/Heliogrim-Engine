#include "LocalPooledAllocator.hpp"

#include "AllocatedMemory.hpp"
#include "AllocationResult.hpp"
#include "GlobalPooledAllocator.hpp"

using namespace ember::engine::gfx::memory;
using namespace ember;

LocalPooledAllocator::LocalPooledAllocator(const ptr<GlobalPooledAllocator> global_) :
    Allocator(),
    _global(global_),
    _cache() {}

LocalPooledAllocator::~LocalPooledAllocator() {
    tidy();
}

void LocalPooledAllocator::tidy() {
    _cache.tidy();
}

AllocationResult LocalPooledAllocator::allocate(cref<MemoryLayout> layout_, const u64 size_,
    ref<ptr<AllocatedMemory>> dst_) {

    if (_cache.allocate(layout_, size_, dst_) == AllocationResult::eSuccess) {
        dst_->allocator = this;
        return AllocationResult::eSuccess;
    }

    return _global->allocate(layout_, size_, dst_);
}

void LocalPooledAllocator::free(mref<ptr<AllocatedMemory>> mem_) {

    if (_cache.free(_STD move(mem_))) {
        return;
    }

    _global->free(_STD move(mem_));
}

#include "../Device/Device.hpp"
#include "../API/VkTranslate.hpp"

AllocationResult engine::gfx::memory::allocate(
    const ptr<LocalPooledAllocator> alloc_,
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
    const ptr<LocalPooledAllocator> alloc_,
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
