#pragma once

#include <Engine.Common/Wrapper.hpp>

#include "__fwd.hpp"

namespace hg::engine::gfx::memory {
    class __declspec(novtable) Allocator {
    public:
        using this_type = Allocator;

    protected:
        Allocator() noexcept = default;

    public:
        virtual ~Allocator() noexcept = default;

    public:
        [[nodiscard]] virtual AllocationResult allocate(
            cref<MemoryLayout> layout_,
            const u64 size_,
            _Out_ ref<uptr<AllocatedMemory>> dst_
        ) = 0;

        virtual void free(mref<uptr<AllocatedMemory>> mem_) = 0;
    };
}
