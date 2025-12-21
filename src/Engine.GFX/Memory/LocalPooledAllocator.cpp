#include "LocalPooledAllocator.hpp"

#include "AllocatedMemory.hpp"
#include "AllocationResult.hpp"
#include "GlobalPooledAllocator.hpp"

using namespace hg::engine::gfx::memory;
using namespace hg;

LocalPooledAllocator::LocalPooledAllocator(const nmpt<GlobalPooledAllocator> global_) noexcept :
	Allocator(),
	_global(global_),
	_cache() {}

LocalPooledAllocator::~LocalPooledAllocator() {
	tidy();
}

void LocalPooledAllocator::tidy() {
	_cache.tidy();
}

AllocationResult LocalPooledAllocator::allocate(
	cref<MemoryLayout> layout_,
	const u64 size_,
	ref<uptr<AllocatedMemory>> dst_
) {

	if (_cache.allocate(layout_, size_, dst_) == AllocationResult::eSuccess) {
		dst_->allocator = this;
		return AllocationResult::eSuccess;
	}

	return _global->allocate(layout_, size_, dst_);
}

void LocalPooledAllocator::free(mref<uptr<AllocatedMemory>> mem_) {

	if (_cache.free(mem_)) {
		return;
	}

	::hg::assertrt(mem_ != nullptr);
	_global->free(std::move(mem_));
}

#include "../Device/Device.hpp"
#include "../API/VkTranslate.hpp"

AllocationResult engine::gfx::memory::allocate(
	const ptr<LocalPooledAllocator> alloc_,
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

	return alloc_->allocate(layout, req.size, dst_);
}

AllocationResult engine::gfx::memory::allocate(
	const ptr<LocalPooledAllocator> alloc_,
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

	return alloc_->allocate(layout, req.size, dst_);
}