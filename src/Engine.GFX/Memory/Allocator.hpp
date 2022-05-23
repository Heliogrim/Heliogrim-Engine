#pragma once

#include "__fwd.hpp"
#include "AllocatedMemory.hpp"

namespace ember::engine::gfx::memory {

    class __declspec(novtable) Allocator {
    public:
        using this_type = Allocator;

    protected:
        Allocator() noexcept = default;

    public:
        virtual ~Allocator() noexcept = default;

    public:
        [[nodiscard]] virtual AllocationResult allocate(cref<MemoryLayout> layout_, const u64 size_,
            _Out_ ref<ptr<AllocatedMemory>> dst_) = 0;

        virtual void free(mref<ptr<AllocatedMemory>> mem_) = 0;
    };
}
