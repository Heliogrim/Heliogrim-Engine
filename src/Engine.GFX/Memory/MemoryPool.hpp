#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Set.hpp>
#include <Engine.Common/Collection/Vector.hpp>

#include "MemoryLayout.hpp"
#include "__fwd.hpp"

namespace hg::engine::gfx::memory {
    class MemoryPool {
    public:
        using this_type = MemoryPool;

    public:
        MemoryPool(cref<MemoryLayout> layout_);

        ~MemoryPool();

    public:
        void tidy();

    private:
        MemoryLayout _layout;

    public:
        [[nodiscard]] cref<MemoryLayout> layout() const noexcept;

    private:
        CompactSet<ptr<AllocatedMemory>> _pooling;
        Vector<ptr<AllocatedMemory>> _memory;

        u64 _totalMemory;
        _STD atomic_uint_fast64_t _totalUsedMemory;
        _STD atomic_uint_fast32_t _totalAlloc;

    public:
        [[nodiscard]] u64 totalMemory() const noexcept;

        [[nodiscard]] u64 usedMemory() const noexcept;

        [[nodiscard]] u64 freeMemory() const noexcept;

        [[nodiscard]] u32 allocCount() const noexcept;

        [[nodiscard]] u32 freeAllocCount() const noexcept;

    public:
        void push(mref<ptr<AllocatedMemory>> memory_);

    private:
        void treeSplice(mref<ptr<AllocatedMemory>> memory_, const u64 targetSize_);

        [[nodiscard]] AllocationResult allocate(
            const u64 size_,
            const bool bestFit_,
            _Out_ ref<ptr<AllocatedMemory>> dst_
        );

    public:
        [[nodiscard]] AllocationResult allocate(const u64 size_, _Out_ ref<ptr<AllocatedMemory>> dst_);

        bool free(mref<ptr<AllocatedMemory>> mem_);
    };
}
