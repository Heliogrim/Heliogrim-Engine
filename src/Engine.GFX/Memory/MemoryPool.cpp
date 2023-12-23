#include "MemoryPool.hpp"

#include <cassert>
#include <Engine.Common/Make.hpp>

#include "AllocatedMemory.hpp"
#include "AllocationResult.hpp"

using namespace hg::engine::gfx::memory;
using namespace hg;

MemoryPool::MemoryPool(cref<MemoryLayout> layout_) :
    _layout(layout_),
    _pooling(),
    _memory(),
    _totalMemory(),
    _totalUsedMemory(),
    _totalAlloc() {}

MemoryPool::~MemoryPool() {
    tidy();
}

void MemoryPool::tidy() {

    for (auto&& entry : _memory) {

        if (entry->parent && poolContains(entry->parent)) {
            //
            entry->allocator = nullptr;
            entry->parent = nullptr;
            //
            entry->vkMemory = nullptr;
        }

        AllocatedMemory::free(_STD move(entry));
    }

    _memory.clear();
    _memory.shrink_to_fit();

    for (const uptr<AllocatedMemory>& entry : _pooling) {

        if (entry->parent && poolContains(entry->parent)) {
            //
            entry->allocator = nullptr;
            entry->parent = nullptr;
            //
            entry->vkMemory = nullptr;
        }

        AllocatedMemory::free(uptr<AllocatedMemory> { const_cast<ref<uptr<AllocatedMemory>>>(entry).release() });
    }

    _pooling.clear();
}

cref<MemoryLayout> MemoryPool::layout() const noexcept {
    return _layout;
}

u64 MemoryPool::totalMemory() const noexcept {
    return _totalMemory;
}

u64 MemoryPool::usedMemory() const noexcept {
    return _totalUsedMemory;
}

u64 MemoryPool::freeMemory() const noexcept {
    return _totalMemory - _totalUsedMemory;
}

u32 MemoryPool::allocCount() const noexcept {
    return _totalAlloc;
}

u32 MemoryPool::freeAllocCount() const noexcept {
    return static_cast<u32>(freeMemory() / _layout.align);
}

void MemoryPool::push(mref<uptr<AllocatedMemory>> memory_) {
    _totalMemory += memory_->size;
    _memory.emplace_back(_STD move(memory_));
}

void MemoryPool::treeSplice(mref<uptr<AllocatedMemory>> memory_, const u64 targetSize_) {

    uptr<AllocatedMemory> next { _STD move(memory_) };
    while (next->size > targetSize_) {

        #if FALSE
        const auto size { next->size >> 2ui64 };
        const auto baseOffset { next->offset };

        const ptr<AllocatedMemory> sparse[] {
            make_ptr<AllocatedMemory>(next->allocator, next, _layout, size,
                baseOffset, next->vkDevice, next->vkMemory, nullptr),
            make_ptr<AllocatedMemory>(next->allocator, next, _layout, size,
                baseOffset + size, next->vkDevice, next->vkMemory, nullptr),
            make_ptr<AllocatedMemory>(next->allocator, next, _layout, size,
                baseOffset + size + size, next->vkDevice, next->vkMemory, nullptr),
            make_ptr<AllocatedMemory>(next->allocator, next, _layout, size,
                baseOffset + size + size + size, next->vkDevice, next->vkMemory, nullptr)
        };

        _memory.push_back(sparse[0]);
        _memory.push_back(sparse[1]);
        _memory.push_back(sparse[2]);

        next = sparse[3];
        #endif

        const auto size { next->size >> 1ui64 };
        const auto baseOffset { next->offset };

        // Prevent to much splitting under target size
        if (size < targetSize_) {
            break;
        }

        nmpt<AllocatedMemory> parent = next.get();
        _pooling.insert(_STD move(next));

        _memory.push_back(
            make_uptr<AllocatedMemory>(
                parent->allocator,
                parent,
                _layout,
                size,
                baseOffset,
                parent->vkDevice,
                parent->vkMemory,
                nullptr
            )
        );

        next = make_uptr<AllocatedMemory>(
            parent->allocator,
            parent,
            _layout,
            size,
            baseOffset + size,
            parent->vkDevice,
            parent->vkMemory,
            nullptr
        );
    }

    _memory.emplace_back(_STD move(next));

    _STD ranges::sort(
        _memory,
        _STD less<u64>(),
        [](const auto& entry_) {
            return entry_->size;
        }
    );
}

AllocationResult MemoryPool::allocate(const u64 size_, const bool bestFit_, ref<uptr<AllocatedMemory>> dst_) {

    auto lbe = _STD ranges::lower_bound(
        _memory,
        size_,
        [](u64 left_, u64 right_) {
            return left_ < right_;
        },
        [](const auto& entry_) {
            return entry_->size;
        }
    );

    if (lbe >= _memory.end()) {
        return AllocationResult::eOutOfMemory;
    }

    if ((*lbe)->size == size_ || (bestFit_ && (*lbe)->size >= size_)) {
        auto mem = _STD move(*lbe);
        _memory.erase(lbe);

        /**
         * Update metrics
         */
        _totalUsedMemory += mem->size;
        ++_totalAlloc;

        /**
         *
         */
        dst_ = _STD move(mem);
        return AllocationResult::eSuccess;
    }

    auto mem = _STD move(*lbe);
    _memory.erase(lbe);

    assert(mem != nullptr);

    treeSplice(_STD move(mem), size_);
    return allocate(size_, true, dst_);
}

AllocationResult MemoryPool::allocate(const u64 size_, ref<uptr<AllocatedMemory>> dst_) {
    return allocate(size_, false, dst_);
}

bool MemoryPool::free(mref<uptr<AllocatedMemory>> mem_) {

    if (mem_->parent && not poolContains(mem_->parent)) {
        return false;
    }

    // Warning: Hotfix
    mem_->allocator = nullptr;

    /**
     * Store memory or restore predecessor memory block if possible
     */
    const auto* const memptr = mem_.get();
    _memory.emplace_back(_STD move(mem_));

    _STD ranges::sort(
        _memory,
        _STD less<u64>(),
        [](const auto& entry_) {
            return entry_->size;
        }
    );

    /**
     * Update metrics
     */
    _totalUsedMemory -= memptr->size;
    --_totalAlloc;

    return true;
}

bool MemoryPool::poolContains(nmpt<AllocatedMemory> allocated_) const noexcept {
    return _STD ranges::contains(
        _pooling,
        allocated_,
        [](const auto& owned_) {
            return nmpt<AllocatedMemory> { owned_.get() };
        }
    );
}
