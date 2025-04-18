﻿#include "GlobalPooledAllocator.hpp"

#include <Engine.Asserts/Breakpoint.hpp>

#include "AllocatedMemory.hpp"
#include "AllocationResult.hpp"
#include "Allocator.hpp"
#include "MemoryPool.hpp"

using namespace hg::engine::gfx::memory;
using namespace hg;

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

	if (size_ == 4096LL || size_ == 65536LL) {
		return true;
	}

	if ((size_ == 64uLL || size_ == 4uLL) && layout_.props & MemoryProperty::eHostVisible) {
		return true;
	}

	return false;
}

AllocationResult GlobalPooledAllocator::allocate(
	cref<MemoryLayout> layout_,
	const u64 size_,
	ref<uptr<AllocatedMemory>> dst_
) {

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
        	::hg::breakpoint();
            return AllocationResult::eFailed;
        }
		#else
		auto* const pool { _cache.getOrCreatePool(layout_) };
		if (pool == nullptr) {
			throw std::runtime_error("Failed to allocate pooled memory.");
			return AllocationResult::eFailed;
		}

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
		(targetSize / layout.align) * layout.align + (targetSize % layout.align ? layout.align : 0uLL)
	};

	// (2^n)-1
	return alignedSize;
}

void GlobalPooledAllocator::augmentPool(ptr<MemoryPool> pool_, const u64 size_) {

	const auto poolSize { nextPoolSize(pool_, size_) };

	uptr<AllocatedMemory> mem { nullptr };
	const auto allocRes { _alloc->allocate(pool_->layout(), poolSize, mem) };

	assert(allocRes == AllocationResult::eSuccess);
	pool_->push(std::move(mem));
}

void GlobalPooledAllocator::free(mref<uptr<AllocatedMemory>> mem_) {

	if (_cache.free(std::move(mem_))) {
		return;
	}

	_alloc->free(std::move(mem_));
}

#include "../Buffer/Buffer.hpp"
#include "../Device/Device.hpp"
#include "../API/VkTranslate.hpp"

AllocationResult engine::gfx::memory::allocate(
	ref<GlobalPooledAllocator> alloc_,
	cref<sptr<Device>> device_,
	cref<MemoryProperties> props_,
	ref<Buffer> buffer_
) {

	const auto req { device_->vkDevice().getBufferMemoryRequirements(buffer_.buffer) };
	const MemoryLayout layout {
		req.alignment,
		props_,
		req.memoryTypeBits
	};

	auto size { req.size };

	const auto atomicNonCoherent = (
		(buffer_.usageFlags & vk::BufferUsageFlagBits::eTransferSrc) &&
		!(props_ & MemoryProperty::eHostCoherent) &&
		(props_ & MemoryProperty::eHostVisible)
	);

	// Warning: Check for flush intent
	if (atomicNonCoherent) {

		// vkPhysicalDeviceLimits::nonCoherentAtomSize <-> Required for flushing
		constexpr auto align { 0x80uLL };
		constexpr auto shift { 7uLL };
		constexpr auto mask { 0b0111'1111uLL };

		const auto aligned {
			((size >> shift) << shift) +
			((size & mask) ? 1uLL << shift : 0uLL)
		};

		size = aligned;

	} else if (size < req.alignment) {

		// Forcefully override if size of lower than base alignment
		size = MAX(req.size, req.alignment);

	}

	return alloc_.allocate(layout, size, buffer_.memory);
}

AllocationResult engine::gfx::memory::allocate(
	ref<GlobalPooledAllocator> alloc_,
	cref<sptr<Device>> device_,
	cref<vk::Buffer> buffer_,
	cref<MemoryProperties> props_,
	ref<uptr<AllocatedMemory>> dst_
) {

	const auto req { device_->vkDevice().getBufferMemoryRequirements(buffer_) };
	const MemoryLayout layout {
		req.alignment,
		props_,
		req.memoryTypeBits
	};

	return alloc_.allocate(layout, req.size, dst_);
}

AllocationResult engine::gfx::memory::allocate(
	ref<GlobalPooledAllocator> alloc_,
	cref<sptr<Device>> device_,
	vk::Image image_,
	cref<MemoryProperties> props_,
	ref<uptr<AllocatedMemory>> dst_
) {

	const auto req { device_->vkDevice().getImageMemoryRequirements(image_) };
	const MemoryLayout layout {
		req.alignment,
		props_,
		req.memoryTypeBits
	};

	return alloc_.allocate(layout, req.size, dst_);
}
