#include "SparseBuffer.hpp"

#include "../Memory/VirtualMemory.hpp"
#include "../Command/CommandQueue.hpp"
#include "SparseBufferView.hpp"

// TODO: Remove
#include "Engine.GFX/Graphics.hpp"
#include <Engine.Core/Engine.hpp>

using namespace hg::engine::gfx;
using namespace hg;

SparseBuffer::SparseBuffer() noexcept :
    _bufferSize(0uLL),
    _memory(nullptr),
    _pages(),
    _vkBuffer(),
    _vkBufferUsageFlags(),
    _changed(false) {}

SparseBuffer::SparseBuffer(
    mref<uptr<VirtualMemory>> memory_,
    u64 bufferSize_,
    cref<vk::Buffer> buffer_,
    cref<vk::BufferUsageFlags> usageFlags_
) noexcept :
    _bufferSize(bufferSize_),
    _memory(std::move(memory_)),
    _pages(),
    _vkBuffer(buffer_),
    _vkBufferUsageFlags(usageFlags_),
    _changed(false) {}

SparseBuffer::SparseBuffer(mref<this_type> other_) noexcept :
    _bufferSize(std::exchange(other_._bufferSize, 0uLL)),
    _memory(std::move(other_._memory)),
    _pages(std::move(other_._pages)),
    _vkBuffer(std::exchange(other_._vkBuffer, {})),
    _vkBufferUsageFlags(std::exchange(other_._vkBufferUsageFlags, {})),
    _changed(std::exchange(other_._changed, false)),
    _bindings(std::move(other_._bindings)),
    _bindData(std::exchange(other_._bindData, {})) {}

SparseBuffer::~SparseBuffer() {
    tidy();
}

ref<SparseBuffer::this_type> SparseBuffer::operator=(mref<this_type> other_) noexcept {
    if (std::addressof(other_) != this) {
        /**
         * Might be equal to `std::swap(*this, other_)`
         */

        _bufferSize = std::exchange(other_._bufferSize, _bufferSize);
        _memory.swap(other_._memory);
        _pages = std::exchange(other_._pages, _pages);
        _vkBuffer = std::exchange(other_._vkBuffer, _vkBuffer);
        _vkBufferUsageFlags = std::exchange(other_._vkBufferUsageFlags, _vkBufferUsageFlags);
        _changed = std::exchange(other_._changed, _changed);
        _bindings = std::exchange(other_._bindings, _bindings);
        _bindData = std::exchange(other_._bindData, _bindData);
    }

    return *this;
}

void SparseBuffer::tidy() {
    /**
     * Destroy Buffer before backing memory or pages
     */
    if (_vkBuffer) {
        // TODO: Check whether virtual buffer should get a reference to the device
        Engine::getEngine()->getGraphics()->getCurrentDevice()->vkDevice().destroyBuffer(_vkBuffer);
        _bufferSize = 0uLL;
    }

    /**
     * Cleanup Pages
     */
    for (auto& entry : _pages) {
        /* Memory Pages are getting freed by follow-up memory destructor */
        entry->release();
        delete entry;
        entry = nullptr;
    }

    /**
     * Cleanup Memory
     */
    _memory.reset();
}

const ptr<const VirtualMemory> SparseBuffer::memory() const noexcept {
    return _memory.get();
}

const ptr<VirtualMemory> SparseBuffer::memory() noexcept {
    return _memory.get();
}

u64 SparseBuffer::size() const noexcept {
    return _bufferSize;
}

u64 SparseBuffer::memorySize() const noexcept {
    return _memory->size();
}

u64 SparseBuffer::memoryResidentialSize() const noexcept {
    return _memory->allocatedSize();
}

cref<vk::Buffer> SparseBuffer::vkBuffer() const noexcept {
    return _vkBuffer;
}

cref<vk::BufferUsageFlags> SparseBuffer::vkBufferUsageFlags() const noexcept {
    return _vkBufferUsageFlags;
}

non_owning_rptr<SparseBufferPage> SparseBuffer::addPage(const u64 size_, const u64 offset_) {
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

    const auto page = new SparseBufferPage {
        memPage,
        size_,
        offset_
    };

    _pages.push_back(page);
    return page;
}

cref<Vector<ptr<SparseBufferPage>>> SparseBuffer::pages() const noexcept {
    return _pages;
}

void SparseBuffer::assureTiledPages(const u64 offset_, const u64 size_) {}

void SparseBuffer::selectPages(
    const u64 offset_,
    const u64 size_,
    ref<Vector<non_owning_rptr<SparseBufferPage>>> pages_
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

uptr<SparseBufferView> SparseBuffer::makeView(const u64 offset_, const u64 size_) {
    assureTiledPages(offset_, size_);

    Vector<non_owning_rptr<SparseBufferPage>> pages {};
    selectPages(offset_, size_, pages);

    /**
     * Take the time to sort the pages by it's mip level (virtual backing)
     */
    std::ranges::sort(
        pages,
        [](non_owning_rptr<SparseBufferPage> left_, non_owning_rptr<SparseBufferPage> right_) {
            return left_->resourceOffset() < right_->resourceOffset();
        }
    );

    /**
     *
     */
    const auto view {
        new SparseBufferView(
            this,
            std::move(pages),
            offset_,
            size_
        )
    };

    return std::unique_ptr<SparseBufferView>(view);
}

void SparseBuffer::updateBindingData() {
    cref<Vector<ptr<SparseBufferPage>>> updates { _pages };
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

void SparseBuffer::enqueueBinding(const ptr<CommandQueue> queue_) {
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

void SparseBuffer::enqueueBinding(
    const ptr<CommandQueue> queue_,
    cref<Vector<vk::Semaphore>> waits_,
    cref<Vector<vk::Semaphore>> signals_
) {
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

void SparseBuffer::enqueueBindingSync(const ptr<CommandQueue> queue_) {
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
