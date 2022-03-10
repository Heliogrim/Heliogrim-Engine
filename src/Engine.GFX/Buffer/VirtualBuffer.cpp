#include "VirtualBuffer.hpp"

#include "../Memory/VirtualMemory.hpp"
#include "../Memory/VkAllocator.hpp"

using namespace ember::engine::gfx;
using namespace ember;

VirtualBuffer::VirtualBuffer() noexcept :
    _allocator(nullptr),
    _memory(nullptr),
    _pages(),
    _vkBuffer(),
    _vkBufferUsageFlags(),
    _changed(false) {}

VirtualBuffer::VirtualBuffer(const ptr<Allocator> allocator_, cref<vk::Buffer> buffer_,
    cref<vk::BufferUsageFlags> usageFlags_) noexcept :
    _allocator(allocator_),
    _memory(nullptr),
    _pages(),
    _vkBuffer(buffer_),
    _vkBufferUsageFlags(usageFlags_),
    _changed(false) {}

VirtualBuffer::~VirtualBuffer() {}

const ptr<const Allocator> VirtualBuffer::allocator() const noexcept {
    return _allocator;
}

ref<ptr<Allocator>> VirtualBuffer::allocator() noexcept {
    return _allocator;
}

const ptr<const VirtualMemory> VirtualBuffer::memory() const noexcept {
    return _memory;
}

const ptr<VirtualMemory> VirtualBuffer::memory() noexcept {
    return _memory;
}

u64 VirtualBuffer::size() const noexcept {
    return _memory->size();
}

u64 VirtualBuffer::residentialSize() const noexcept {
    return _memory->allocatedSize();
}

cref<vk::Buffer> VirtualBuffer::vkBuffer() const noexcept {
    return _vkBuffer;
}

cref<vk::BufferUsageFlags> VirtualBuffer::vkBufferUsageFlags() const noexcept {
    return _vkBufferUsageFlags;
}

non_owning_rptr<VirtualBufferPage> VirtualBuffer::addPage(const u64 size_, const u64 offset_) {

    #ifdef _DEBUG
    for (const auto& entry : _pages) {
        if (entry->resourceOffset() >= (offset_ + size_)) {
            continue;
        }

        const auto eo { entry->resourceOffset() };
        const auto es { entry->resourceSize() };

        assert((eo + es) <= offset_);
    }
    #endif

    const auto memPage = _memory->definePage(size_);
    assert(memPage);

    const auto page = new VirtualBufferPage {
        memPage,
        size_,
        offset_
    };

    _pages.push_back(page);
    return page;
}

void VirtualBuffer::updateBindingData() {

    Vector<ptr<VirtualBufferPage>> updates { _pages };
    for (const auto& page : updates) {
        _bindings.push_back(page->vkSparseMemoryBind());
    }

    /**
     *
     */
    _bindData = vk::SparseBufferMemoryBindInfo {
        _vkBuffer,
        static_cast<u32>(_bindings.size()),
        _bindings.data()
    };
}
