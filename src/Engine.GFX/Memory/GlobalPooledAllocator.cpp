#include "GlobalPooledAllocator.hpp"

#include "AllocationResult.hpp"
#include "Allocator.hpp"
#include "MemoryPool.hpp"

using namespace ember::engine::gfx::memory;
using namespace ember;

GlobalPooledAllocator::GlobalPooledAllocator(cref<sptr<Allocator>> nativeAllocator_) :
    Allocator(),
    _alloc(nativeAllocator_) {}

GlobalPooledAllocator::~GlobalPooledAllocator() {
    tidy();
}

void GlobalPooledAllocator::tidy() {
    _cache.tidy();
}

bool GlobalPooledAllocator::shouldPool(cref<MemoryLayout> layout_, const u64 size_) const noexcept {

    if (size_ == 4096i64 || size_ == 65536i64) {
        return true;
    }

    return false;
}

AllocationResult GlobalPooledAllocator::allocate(cref<MemoryLayout> layout_, const u64 size_,
    ref<ptr<AllocatedMemory>> dst_) {

    const auto cacheResult { _cache.allocate(layout_, size_, dst_) };
    if (cacheResult == AllocationResult::eSuccess) {
        dst_->allocator = this;
        return AllocationResult::eSuccess;
    }

    /**
     *
     */
    if (shouldPool(layout_, size_)) {
        #ifdef _DEBUG
        auto* const pool { _cache.getOrCreatePool(layout_) };
        if (pool == nullptr) {
            __debugbreak();
            return AllocationResult::eFailed;
        }
        #else
        throw runtime_error("Failed to allocate pooled memory.");
        #endif

        /**
         *
         */
        augmentPool(pool, size_);

        /**
         *
         */
        const auto result { _cache.allocate(layout_, size_, dst_) };
        if (result == eSuccess) {
            dst_->allocator = this;
        }

        return result;
    }

    /**
     *
     */
    return _alloc->allocate(layout_, size_, dst_);
}

u64 GlobalPooledAllocator::nextPoolSize(const ptr<const MemoryPool> pool_, const u64 requestedSize_) const noexcept {

    const auto& layout { pool_->layout() };
    const auto totalSize { pool_->totalMemory() };

    const auto targetSize { totalSize < requestedSize_ ? requestedSize_ : totalSize };
    const auto alignedSize {
        (targetSize / layout.align) * layout.align + (targetSize % layout.align ? layout.align : 0ui64)
    };

    // (2^n)-1
    return alignedSize;
}

void GlobalPooledAllocator::augmentPool(ptr<MemoryPool> pool_, const u64 size_) {

    const auto poolSize { nextPoolSize(pool_, size_) };

    ptr<AllocatedMemory> mem { nullptr };
    const auto allocRes { _alloc->allocate(pool_->layout(), poolSize, mem) };

    assert(allocRes == AllocationResult::eSuccess);
    pool_->push(_STD move(mem));
}

void GlobalPooledAllocator::free(mref<ptr<AllocatedMemory>> mem_) {

    if (_cache.free(_STD move(mem_))) {
        return;
    }

    _alloc->free(_STD move(mem_));
}

#include "../Buffer/Buffer.hpp"
#include "../Device/Device.hpp"
#include "../API/VkTranslate.hpp"

AllocationResult engine::gfx::memory::allocate(const ptr<GlobalPooledAllocator> alloc_, cref<sptr<Device>> device_,
    cref<MemoryProperties> props_, ref<Buffer> buffer_) {

    const auto req { device_->vkDevice().getBufferMemoryRequirements(buffer_.buffer) };
    const MemoryLayout layout {
        req.alignment,
        props_,
        req.memoryTypeBits
    };

    auto size { req.size };

    // Warning: Check for flush intent
    if (
        buffer_.usageFlags & vk::BufferUsageFlagBits::eTransferSrc &&
        !(props_ & MemoryProperty::eHostCoherent) &&
        props_ & MemoryProperty::eHostVisible
    ) {

        // vkPhysicalDeviceLimits::nonCoherentAtomSize <-> Required for flushing
        constexpr auto align { 0x80ui64 };
        constexpr auto shift { 7ui64 };
        constexpr auto mask { 0b0111'1111ui64 };

        const auto aligned {
            ((size >> shift) << shift) +
            ((size & mask) ? 1ui64 << shift : 0ui64)
        };

        size = aligned;
    }

    return alloc_->allocate(layout, size, buffer_.memory);
}

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
