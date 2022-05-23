#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>

#include "MemoryLayout.hpp"
#include "__fwd.hpp"

namespace ember::engine::gfx::memory {

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
        Vector<ptr<AllocatedMemory>> _memory;

    public:
        [[nodiscard]] AllocationResult allocate(const u64 size_, _Out_ ref<ptr<AllocatedMemory>> dst_);

        bool free(mref<ptr<AllocatedMemory>> mem_);
    };
}
