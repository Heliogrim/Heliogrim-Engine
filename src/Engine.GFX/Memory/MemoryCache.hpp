#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/BytellHashMap.hpp>

#include "__fwd.hpp"
#include "MemoryLayout.hpp"

namespace hg::engine::gfx::memory {
    class MemoryCache {
    public:
        using this_type = MemoryCache;

    public:
        MemoryCache();

        ~MemoryCache();

    public:
        void tidy();

    private:
        ska::bytell_hash_map<MemoryLayout, ptr<MemoryPool>> _pools;

    public:
        [[nodiscard]] non_owning_rptr<MemoryPool> getOrCreatePool(cref<MemoryLayout> layout_);

        [[nodiscard]] AllocationResult allocate(
            cref<MemoryLayout> layout_,
            const u64 size_,
            _Out_ ref<ptr<AllocatedMemory>> dst_
        );

        bool free(mref<ptr<AllocatedMemory>> mem_);
    };
}
