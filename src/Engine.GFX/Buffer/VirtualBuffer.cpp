#include "VirtualBuffer.hpp"

#include "../Memory/VirtualMemory.hpp"
#include "../Memory/VkAllocator.hpp"
#include "../Command/CommandQueue.hpp"

using namespace ember::engine::gfx;
using namespace ember;

VirtualBuffer::VirtualBuffer() noexcept :
    _allocator(nullptr),
    _memory(nullptr),
    _pages(),
    _vkBuffer(),
    _vkBufferUsageFlags(),
    _changed(false) {}

VirtualBuffer::VirtualBuffer(const ptr<memory::Allocator> allocator_, cref<vk::Buffer> buffer_,
    cref<vk::BufferUsageFlags> usageFlags_) noexcept :
    _allocator(allocator_),
    _memory(nullptr),
    _pages(),
    _vkBuffer(buffer_),
    _vkBufferUsageFlags(usageFlags_),
    _changed(false) {}

VirtualBuffer::VirtualBuffer(mref<this_type> other_) noexcept :
    _allocator(other_._allocator),
    _memory(_STD exchange(other_._memory, nullptr)),
    _pages(_STD move(other_._pages)),
    _vkBuffer(_STD exchange(other_._vkBuffer, {})),
    _vkBufferUsageFlags(_STD exchange(other_._vkBufferUsageFlags, {})),
    _changed(_STD exchange(other_._changed, false)),
    _bindings(_STD move(other_._bindings)),
    _bindData(_STD exchange(other_._bindData, {})) {}

VirtualBuffer::~VirtualBuffer() {
    tidy();
}

ref<VirtualBuffer::this_type> VirtualBuffer::operator=(mref<this_type> other_) noexcept {

    if (_STD addressof(other_) != this) {
        /**
         * Might be equal to `_STD swap(*this, other_)`
         */
        _allocator = _STD exchange(other_._allocator, _allocator);
        _memory = _STD exchange(other_._memory, _memory);
        _pages = _STD exchange(other_._pages, _pages);
        _vkBuffer = _STD exchange(other_._vkBuffer, _vkBuffer);
        _vkBufferUsageFlags = _STD exchange(other_._vkBufferUsageFlags, _vkBufferUsageFlags);
        _changed = _STD exchange(other_._changed, _changed);
        _bindings = _STD exchange(other_._bindings, _bindings);
        _bindData = _STD exchange(other_._bindData, _bindData);
    }

    return *this;
}

void VirtualBuffer::tidy() {

    /**
     * Cleanup Pages
     */
    for (auto& entry : _pages) {
        delete entry;
        entry = nullptr;
    }

    /**
     * Cleanup Memory
     */
    if (_memory != nullptr) {
        delete _memory;
        _memory = nullptr;
    }

    /**
     * Destroy Buffer
     */
    if (_vkBuffer) {
        // TODO:
        __debugbreak();
    }
}

const ptr<const memory::Allocator> VirtualBuffer::allocator() const noexcept {
    return _allocator;
}

ref<ptr<memory::Allocator>> VirtualBuffer::allocator() noexcept {
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

    cref<Vector<ptr<VirtualBufferPage>>> updates { _pages };
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

void VirtualBuffer::enqueueBinding(const ptr<CommandQueue> queue_) {

    vk::BindSparseInfo bsi {
        0,
        nullptr,
        1ui32,
        &_bindData,
        0,
        nullptr,
        0,
        nullptr,
        0,
        nullptr,
        nullptr
    };

    #ifdef _DEBUG
    const auto res { queue_->vkQueue().bindSparse(1, &bsi, nullptr) };
    assert(res == vk::Result::eSuccess);
    #else
    [[maybe_unused]] const auto res { queue_->vkQueue().bindSparse(1, &bsi, nullptr) };
    #endif
}

void VirtualBuffer::enqueueBinding(const ptr<CommandQueue> queue_, cref<Vector<vk::Semaphore>> waits_,
    cref<Vector<vk::Semaphore>> signals_) {

    vk::BindSparseInfo bsi {
        static_cast<u32>(waits_.size()),
        waits_.data(),
        1ui32,
        &_bindData,
        0,
        nullptr,
        0,
        nullptr,
        static_cast<u32>(signals_.size()),
        signals_.data(),
        nullptr
    };

    #ifdef _DEBUG
    const auto res { queue_->vkQueue().bindSparse(1, &bsi, nullptr) };
    assert(res == vk::Result::eSuccess);
    #else
    [[maybe_unused]] const auto res { queue_->vkQueue().bindSparse(1, &bsi, nullptr) };
    #endif

}
