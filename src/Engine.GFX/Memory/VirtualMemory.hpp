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
            nmpt<memory::GlobalPooledAllocator> allocator_,
            cref<memory::MemoryLayout> layout_,
            const u64 size_
        );

        ~VirtualMemory();

    private:
        nmpt<memory::GlobalPooledAllocator> _allocator;

    public:
        [[nodiscard]] nmpt<memory::GlobalPooledAllocator> allocator() const noexcept;

    private:
        memory::MemoryLayout _layout;

    public:
        [[nodiscard]] cref<memory::MemoryLayout> layout() const noexcept;

    private:
        Vector<uptr<VirtualMemoryPage>> _pages;

    public:
        [[nodiscard]] nmpt<VirtualMemoryPage> definePage(const u64 offset_, const u64 size_);

        void undefinePage(mref<nmpt<VirtualMemoryPage>> page_);

    private:
        u64 _size;

    public:
        [[nodiscard]] u64 size() const noexcept;

        [[nodiscard]] u64 allocatedSize() const noexcept;

        [[nodiscard]] u64 pageCount() const noexcept;
    };
}
