#pragma once
#include <Engine.Common/Collection/Vector.hpp>

#include "Allocator.hpp"
#include "VirtualMemoryPage.hpp"

namespace ember::engine::gfx {

    class VirtualMemory {
    public:
        using this_type = VirtualMemory;

    public:
        VirtualMemory(const ptr<Allocator> allocator_);

        ~VirtualMemory();

    private:
        ptr<Allocator> _allocator;
        Vector<ptr<VirtualMemoryPage>> _pages;

    public:
        [[nodiscard]] non_owning_rptr<VirtualMemoryPage> definePage(const u64 size_);

        void undefinePage(const non_owning_rptr<VirtualMemoryPage> page_);

    public:
        [[nodiscard]] u64 size() const noexcept;

        [[nodiscard]] u64 allocatedSize() const noexcept;

        [[nodiscard]] u64 pageCount() const noexcept;
    };
}
