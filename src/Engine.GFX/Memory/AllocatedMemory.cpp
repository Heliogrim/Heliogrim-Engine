#include "AllocatedMemory.hpp"

#include <cassert>
#include <cstring>
#include <utility>
#include <Engine.Common/Memory/MemoryPointer.hpp>

#include "Allocator.hpp"

using namespace hg::engine::gfx::memory;
using namespace hg;

AllocatedMemory::AllocatedMemory(
	nmpt<Allocator> allocator_,
	nmpt<AllocatedMemory> parent_,
	cref<MemoryLayout> layout_,
	cref<u64> size_,
	cref<u64> offset_,
	cref<vk::Device> vkDevice_,
	cref<vk::DeviceMemory> vkMemory_,
	MemoryMapping mapping_
) noexcept :
	allocator(allocator_),
	parent(parent_),
	layout(layout_),
	size(size_),
	offset(offset_),
	vkDevice(vkDevice_),
	vkMemory(vkMemory_),
	mapping(mapping_) {}

AllocatedMemory::AllocatedMemory(mref<AllocatedMemory> other_) noexcept :
	allocator(std::exchange(other_.allocator, nullptr)),
	parent(std::exchange(other_.parent, nullptr)),
	layout(std::exchange(other_.layout, {})),
	size(std::exchange(other_.size, 0)),
	offset(std::exchange(other_.offset, 0)),
	vkDevice(std::exchange(other_.vkDevice, nullptr)),
	vkMemory(std::exchange(other_.vkMemory, nullptr)),
	mapping(std::exchange(other_.mapping, nullptr)) {}

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
		throw std::runtime_error("Possible memory leak due to missing hierarchical release report.");
	}
	#endif

	if (vkMemory && !parent && !allocator) {
		vkDevice.freeMemory(vkMemory);
	}
}

ref<AllocatedMemory> AllocatedMemory::operator=(mref<AllocatedMemory> other_) noexcept {

	if (std::addressof(other_) != this) {
		std::swap(allocator, other_.allocator);
		std::swap(parent, other_.parent);
		std::swap(layout, other_.layout);
		std::swap(size, other_.size);
		std::swap(mapping, other_.mapping);
		std::swap(vkMemory, other_.vkMemory);
		std::swap(vkDevice, other_.vkDevice);
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

	std::memcpy(mapping, data_, static_cast<size_t>(size_));
	return true;
}

void AllocatedMemory::free(mref<uptr<AllocatedMemory>> memory_) {

	if (memory_->mapping) {
		memory_->unmap();
	}

	if (memory_->allocator) {
		return memory_->allocator->free(std::move(memory_));
	}

	memory_.reset();
}
