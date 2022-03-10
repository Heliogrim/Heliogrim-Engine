#pragma once

#include "../Memory/Allocator.hpp"
#include "../Memory/VirtualMemoryPage.hpp"

namespace ember::engine::gfx {

    class VirtualBufferPage final {
    public:
        VirtualBufferPage(const ptr<VirtualMemoryPage> memory_, const u64 resourceSize_, const u64 resourceOffset_);

        ~VirtualBufferPage();

    private:
        ptr<VirtualMemoryPage> _memory;

    public:
        [[nodiscard]] const ptr<VirtualMemoryPage> memory() const noexcept;

        [[nodiscard]] bool resident() const noexcept;

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
