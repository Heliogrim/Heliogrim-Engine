#include "MemoryPool.hpp"

#include <cassert>
#include <ranges>
#include <Engine.Common/Make.hpp>
#include <Engine.Logging/Logger.hpp>

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

    while (not _memory.empty()) {
        auto entry = std::move(_memory.back());
        _memory.pop_back();

        if (
            entry->parent &&
            poolContains(entry->parent) &&
            free(std::move(entry))
        ) {
            __noop();
        } else {
            AllocatedMemory::free(std::move(entry));
        }
    }

    _memory.clear();
    _memory.shrink_to_fit();

    for (const uptr<AllocatedMemory>& entry : _pooling | std::ranges::views::values) {

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

    assert(memory_->parent == nullptr && memory_->allocator != nullptr);

    _totalMemory += memory_->size;

    auto ube = std::ranges::upper_bound(
        _memory,
        memory_->size,
        std::greater_equal<u64>(),
        [](const auto& entry_) {
            return entry_->size;
        }
    );
    _memory.insert(ube, std::move(memory_));
}

bool MemoryPool::treeMerge(
    cref<decltype(_memory)::iterator> begin_,
    cref<decltype(_memory)::iterator> end_,
    ref<uptr<AllocatedMemory>> memory_
) {

    /* Pre-sort memory within same size region based on offset */

    std::ranges::sort(
        begin_,
        end_,
        std::less<u64>(),
        [](const auto& entry_) {
            return entry_->offset;
        }
    );

    /**/

    for (auto it = begin_; it != end_; ++it) {

        const auto diff = std::abs(static_cast<s64>((*it)->offset) - static_cast<s64>(memory_->offset));
        if (diff > memory_->size) {

            if ((*it)->offset > memory_->offset) {
                return false;
            }

            continue;
        }

        /* Found coalescing neighbor candidate */

        if ((*it)->parent == memory_->parent && (*it)->size + memory_->size == memory_->parent->size) {

            assert((*it)->offset == memory_->parent->offset || memory_->offset == memory_->parent->offset);

            auto parentIt = _pooling.find(memory_->parent.get());
            auto hold = std::move(parentIt->second);

            releaseSubAllocation(*it);
            releaseSubAllocation(memory_);

            _pooling.erase(parentIt);
            _memory.erase(it);
            memory_.reset();

            /* Recursive call to free tree-spliced memory */

            const auto result = free(std::move(hold), true);
            if (not result && hold->allocator) {
                memory::AllocatedMemory::free(std::move(hold));

                if (_pooling.empty() && _memory.empty()) {
                    _totalAlloc.store(0);
                    _totalMemory = 0uLL;
                    _totalUsedMemory.store(0);
                }
            }
            return true;
        }
    }

    return false;
}

void MemoryPool::treeSplice(mref<uptr<AllocatedMemory>> memory_, const u64 targetSize_) {

    uptr<AllocatedMemory> next { std::move(memory_) };
    while (next->size > targetSize_) {

        const auto size { next->size >> 1ui64 };
        const auto baseOffset { next->offset };

        // Prevent too much splitting under target size
        if (size < targetSize_) {
            break;
        }

        nmpt<AllocatedMemory> parent = next.get();
        assert(not _pooling.contains(next.get()));
        _pooling.insert(std::make_pair(next.get(), std::move(next)));

        storeSubAllocate(
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

    storeSubAllocate(std::move(next));
}

void MemoryPool::storeSubAllocate(mref<uptr<AllocatedMemory>> memory_) {

    assert(memory_->allocator != nullptr && memory_->parent != nullptr);

    const auto lbe = std::ranges::lower_bound(
        _memory,
        memory_->size,
        std::greater_equal<u64>(),
        [](const auto& entry_) {
            return entry_->size;
        }
    );
    _memory.insert(lbe, std::move(memory_));
}

void MemoryPool::releaseSubAllocation(cref<uptr<AllocatedMemory>> memory_) const {

    assert(memory_->parent && memory_->allocator != nullptr);

    memory_->allocator = nullptr;
    memory_->parent = nullptr;
    memory_->vkMemory = nullptr;
}

AllocationResult MemoryPool::allocate(const u64 size_, const bool bestFit_, ref<uptr<AllocatedMemory>> dst_) {

    auto lbe = std::ranges::lower_bound(
        _memory.rbegin(),
        _memory.rend(),
        size_,
        std::less<u64>(),
        [](const auto& entry_) {
            return entry_->size;
        }
    );

    if (lbe == _memory.rend()) {
        return AllocationResult::eOutOfMemory;
    }

    if ((*lbe)->size == size_ || (bestFit_ && (*lbe)->size >= size_)) {
        auto mem = std::move(*lbe);
        _memory.erase(lbe.base() - 1LL);

        /**
         * Update metrics
         */
        _totalUsedMemory += mem->size;
        ++_totalAlloc;

        /**
         *
         */
        dst_ = std::move(mem);
        return AllocationResult::eSuccess;
    }

    auto mem = std::move(*lbe);
    _memory.erase(lbe.base() - 1LL);

    assert(mem != nullptr);

    treeSplice(std::move(mem), size_);
    return allocate(size_, true, dst_);
}

bool MemoryPool::free(mref<uptr<AllocatedMemory>> mem_, bool cascade_) {
    if (not mem_->parent) {
        return false;
    }

    if (not poolContains(mem_->parent)) {
        return false;
    }

    /**
     * Store memory or restore predecessor memory block if possible
     */
    auto lbe = std::ranges::lower_bound(
        _memory,
        mem_->size,
        std::greater_equal<u64>(),
        [](const auto& entry_) {
            return entry_->size;
        }
    );

    auto ube = std::ranges::upper_bound(
        _memory,
        mem_->size,
        std::greater_equal<u64>(),
        [](const auto& entry_) {
            return entry_->size;
        }
    );

    /**
     * Update metrics
     */
    _totalUsedMemory -= mem_->size;
    --_totalAlloc;

    /**/

    /* Warning: After successful treeMerge call the iterators are invalid to use */

    if (
        mem_->parent &&
        lbe != ube &&
        std::ranges::distance(ube, lbe) > 0LL &&
        treeMerge(ube, lbe, mem_)
    ) {
        __noop();
    } else {
        _memory.insert(ube, std::move(mem_));
    }

    return true;
}

AllocationResult MemoryPool::allocate(const u64 size_, ref<uptr<AllocatedMemory>> dst_) {
    return allocate(size_, false, dst_);
}

bool MemoryPool::free(mref<uptr<AllocatedMemory>> mem_) {

    const auto result = free(std::move(mem_), true);

    if (_pooling.empty() && _memory.empty()) {
        _totalAlloc.store(0);
        _totalMemory = 0uLL;
        _totalUsedMemory.store(0);
    }

    return result;
}

bool MemoryPool::poolContains(nmpt<AllocatedMemory> allocated_) const noexcept {

    #ifdef _DEBUG
    if (not _pooling.contains(allocated_.get())) {
        if (std::ranges::contains(
            _memory,
            allocated_.get(),
            [](const auto& e_) {
                return e_.get();
            }
        )) {
            __debugbreak();
            IM_CORE_ERRORF(
                "Encountered double-free on allocated memory from pool with parent ({:x})",
                reinterpret_cast<ptrdiff_t>(allocated_.get())
            );
        }
    }
    #endif

    return _pooling.contains(allocated_.get());
}
