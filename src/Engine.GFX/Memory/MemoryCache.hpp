#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/DenseMap.hpp>

#include "__fwd.hpp"
#include "MemoryLayout.hpp"

namespace hg::engine::gfx::memory {
    class MemoryCache {
    public:
        using this_type = MemoryCache;

    public:
        MemoryCache() noexcept;

        ~MemoryCache();

    public:
        void tidy();

    private:
        DenseMap<MemoryLayout, ptr<MemoryPool>> _pools;

    public:
        [[nodiscard]] non_owning_rptr<MemoryPool> getOrCreatePool(cref<MemoryLayout> layout_);

        [[nodiscard]] AllocationResult allocate(
            cref<MemoryLayout> layout_,
            const u64 size_,
            _Out_ ref<uptr<AllocatedMemory>> dst_
        );

        bool free(mref<uptr<AllocatedMemory>> mem_);
    };
}
