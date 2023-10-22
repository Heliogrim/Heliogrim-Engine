#pragma once
#include "RenderCommandAllocator.hpp"
#include "RenderCommandPool.hpp"

namespace hg::engine::gfx::render::cmd {
    class PooledRenderCommandAllocator final :
        public RenderCommandAllocator {
    public:
        friend class RenderCommandPool;

    public:
        using this_type = PooledRenderCommandAllocator;

    public:
        PooledRenderCommandAllocator() noexcept = delete;

        PooledRenderCommandAllocator(
            ptr<RenderCommandPool> pool_,
            RenderCommandPool::pool_buffer_identifier identifier_,
            ptr<Vector<ptr<const RenderCommand>>> cmdRecList_,
            ptr<RenderCommandPool::PooledMemory> initial_
        ) noexcept;

        PooledRenderCommandAllocator(mref<this_type> other_) noexcept;

        ~PooledRenderCommandAllocator() noexcept override;

    public:
        ref<this_type> operator=(mref<this_type>) noexcept = delete;

        ref<this_type> operator=(cref<this_type>) = delete;

    protected:
        RenderCommandPool::pool_buffer_identifier _identifier;
        ptr<RenderCommandPool> _pool;

        ptr<Vector<ptr<const RenderCommand>>> _cmdRecList;
        ptr<RenderCommandPool::PooledMemory> _poolMemory;

    public:
        [[nodiscard]] tl::expected<std::span<_::byte>, AllocationError> allocateUnique(const size_type size_) override;

        [[nodiscard]] bool deallocateUnique(mref<std::span<_::byte>> memory_) override;
    };
}
