#pragma once

#include "../vkinc.hpp"

#include "../Memory/Allocator.hpp"
#include "../Memory/VirtualMemoryPage.hpp"

namespace hg::engine::gfx {
    class SparseBufferPage final {
    public:
        SparseBufferPage(nmpt<VirtualMemoryPage> memory_, const u64 resourceSize_, const u64 resourceOffset_);

        ~SparseBufferPage();

    private:
        nmpt<VirtualMemoryPage> _memory;

    public:
        [[nodiscard]] nmpt<VirtualMemoryPage> release() noexcept;

        [[nodiscard]] nmpt<VirtualMemoryPage> memory() const noexcept;

        [[nodiscard]] bool residential() const noexcept;

        bool load();

        bool unload();

    private:
        u64 _resourceSize;
        u64 _resourceOffset;

    public:
        [[nodiscard]] vk::SparseMemoryBind vkSparseMemoryBind() const noexcept;

        [[nodiscard]] u64 resourceSize() const noexcept;

        [[nodiscard]] u64 resourceOffset() const noexcept;
    };
}
