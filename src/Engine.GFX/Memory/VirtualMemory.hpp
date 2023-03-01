#pragma once
#include <Engine.Common/Collection/Vector.hpp>

#include "GlobalPooledAllocator.hpp"
#include "VirtualMemoryPage.hpp"

namespace hg::engine::gfx {
    class VirtualMemory {
    public:
        using this_type = VirtualMemory;

    public:
        VirtualMemory(
            const ptr<memory::GlobalPooledAllocator> allocator_,
            cref<memory::MemoryLayout> layout_,
            const u64 size_
        );

        ~VirtualMemory();

    private:
        ptr<memory::GlobalPooledAllocator> _allocator;

    public:
        [[nodiscard]] ptr<memory::GlobalPooledAllocator> allocator() const noexcept;

    private:
        memory::MemoryLayout _layout;

    public:
        [[nodiscard]] cref<memory::MemoryLayout> layout() const noexcept;

    private:
        Vector<ptr<VirtualMemoryPage>> _pages;

    public:
        [[nodiscard]] non_owning_rptr<VirtualMemoryPage> definePage(const u64 offset_, const u64 size_);

        void undefinePage(const non_owning_rptr<VirtualMemoryPage> page_);

    private:
        u64 _size;

    public:
        [[nodiscard]] u64 size() const noexcept;

        [[nodiscard]] u64 allocatedSize() const noexcept;

        [[nodiscard]] u64 pageCount() const noexcept;
    };
}
