#pragma once

#include <Engine.Common/Collection/DenseMap.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <tl/expected.hpp>

#include "Buffer/RenderCommandBufferView.hpp"

namespace hg::engine::gfx::render::cmd {
    class PooledRenderCommandBuffer;
}

namespace hg::engine::gfx::render::cmd {
    class PoolingError : _STD exception {};

    class RenderCommandPool {
    public:
        friend class PooledRenderCommandAllocator;

    public:
        using this_type = RenderCommandPool;

        using pool_buffer_identifier = size_t;
        using size_type = size_t;

        constexpr static size_type resource_base_size = 1024uLL;

    public:
        RenderCommandPool() noexcept = default;

        ~RenderCommandPool() noexcept = default;

    private:
        _STD atomic_flag _locked;
        _STD atomic<pool_buffer_identifier> _ids;

    private:
        using PooledMemory = _STD pmr::memory_resource;

        struct PooledMemoryResource {
            pool_buffer_identifier acquirer;
            uptr<PooledMemory> memory;
        };

        DenseMap<pool_buffer_identifier, uptr<Vector<ptr<const RenderCommand>>>> _cmdLists;
        Vector<PooledMemoryResource> _segments;

    protected:
        [[nodiscard]] ptr<PooledMemory> nextAllocationSegment(const pool_buffer_identifier identifier_);

        void resetAllocationSegments(const pool_buffer_identifier identifier_);

        void erase(const pool_buffer_identifier identifier_);

    public:
        [[nodiscard]] tl::expected<uptr<PooledRenderCommandBuffer>, PoolingError> acquire() noexcept;

        tl::expected<decltype(true), PoolingError> release(
            mref<uptr<PooledRenderCommandBuffer>> resource_
        ) noexcept;

        [[nodiscard]] tl::expected<RenderCommandBufferView, PoolingError> view(
            const ptr<const PooledRenderCommandBuffer> buffer_
        ) const noexcept;
    };
}
