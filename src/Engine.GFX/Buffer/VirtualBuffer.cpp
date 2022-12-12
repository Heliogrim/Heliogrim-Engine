#include "VirtualBuffer.hpp"

#include "../Memory/VirtualMemory.hpp"
#include "../Command/CommandQueue.hpp"
#include "VirtualBufferView.hpp"

// TODO: Remove
#include "Engine.GFX/Graphics.hpp"
#include <Engine.Core/Engine.hpp>

using namespace ember::engine::gfx;
using namespace ember;

VirtualBuffer::VirtualBuffer() noexcept :
    _memory(nullptr),
    _pages(),
    _vkBuffer(),
    _vkBufferUsageFlags(),
    _changed(false) {}

VirtualBuffer::VirtualBuffer(mref<uptr<VirtualMemory>> memory_, cref<vk::Buffer> buffer_,
    cref<vk::BufferUsageFlags> usageFlags_) noexcept :
    _memory(_STD move(memory_)),
    _pages(),
    _vkBuffer(buffer_),
    _vkBufferUsageFlags(usageFlags_),
    _changed(false) {}

VirtualBuffer::VirtualBuffer(mref<this_type> other_) noexcept :
    _memory(_STD move(other_._memory)),
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

        _memory.swap(other_._memory);
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
     * Destroy Buffer before backing memory or pages
     */
    if (_vkBuffer) {
        // TODO: Check whether virtual buffer should get a reference to the device
        Engine::getEngine()->getGraphics()->getCurrentDevice()->vkDevice().destroyBuffer(_vkBuffer);
    }

    /**
     * Cleanup Pages
     */
    for (auto& entry : _pages) {
        // Unhook paged memory
        _memory->undefinePage(entry->memory());

        delete entry;
        entry = nullptr;
    }

    /**
     * Cleanup Memory
     */
    _memory.reset();
}

const ptr<const VirtualMemory> VirtualBuffer::memory() const noexcept {
    return _memory.get();
}

const ptr<VirtualMemory> VirtualBuffer::memory() noexcept {
    return _memory.get();
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

    // Warning: Replace virtual memory page offset
    const auto memPage = _memory->definePage(0ui64, size_);
    assert(memPage);

    const auto page = new VirtualBufferPage {
        memPage,
        size_,
        offset_
    };

    _pages.push_back(page);
    return page;
}

cref<Vector<ptr<VirtualBufferPage>>> VirtualBuffer::pages() const noexcept {
    return _pages;
}

void VirtualBuffer::assureTiledPages(const u64 offset_, const u64 size_) {}

void VirtualBuffer::selectPages(
    const u64 offset_,
    const u64 size_,
    ref<Vector<non_owning_rptr<VirtualBufferPage>>> pages_
) {
    const u64 lowerBound { offset_ };
    const u64 upperBound { offset_ + size_ };

    for (auto* const entry : _pages) {
        const u64 pageUpperBound { entry->resourceOffset() + entry->resourceSize() };

        if (entry->resourceOffset() > upperBound || lowerBound > pageUpperBound) {
            continue;
        }

        pages_.push_back(entry);
    }
}

uptr<VirtualBufferView> VirtualBuffer::makeView(const u64 offset_, const u64 size_) {
    assureTiledPages(offset_, size_);

    Vector<non_owning_rptr<VirtualBufferPage>> pages {};
    selectPages(offset_, size_, pages);

    /**
     * Take the time to sort the pages by it's mip level (virtual backing)
     */
    _STD ranges::sort(pages, [](non_owning_rptr<VirtualBufferPage> left_, non_owning_rptr<VirtualBufferPage> right_) {
        return left_->resourceOffset() < right_->resourceOffset();
    });

    /**
     *
     */
    const auto view {
        new VirtualBufferView(
            this,
            _STD move(pages),
            offset_,
            size_
        )
    };

    return _STD unique_ptr<VirtualBufferView>(view);
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

void VirtualBuffer::enqueueBindingSync(const ptr<CommandQueue> queue_) {
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

    auto fence { queue_->device()->vkDevice().createFence(vk::FenceCreateInfo {}) };

    #ifdef _DEBUG
    auto res { queue_->vkQueue().bindSparse(1, &bsi, fence) };
    assert(res == vk::Result::eSuccess);
    #else
    [[maybe_unused]] auto res { queue_->vkQueue().bindSparse(1, &bsi, fence) };
    #endif

    res = queue_->device()->vkDevice().waitForFences(1ui32, &fence, VK_TRUE, UINT64_MAX);
    assert(res == vk::Result::eSuccess);
}
