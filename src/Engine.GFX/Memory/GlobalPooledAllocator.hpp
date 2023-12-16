#pragma once

#include <Engine.Common/Wrapper.hpp>

#include "Allocator.hpp"
#include "MemoryCache.hpp"
#include "__fwd.hpp"

namespace hg::engine::gfx::memory {
    class GlobalPooledAllocator final :
        public Allocator {
    public:
        using this_type = GlobalPooledAllocator;

    public:
        GlobalPooledAllocator(cref<sptr<Allocator>> nativeAllocator_);

        ~GlobalPooledAllocator() override;

    public:
        void tidy();

    private:
        sptr<Allocator> _alloc;

    private:
        MemoryCache _cache;

    private:
        /**
         * Query whether the given layout and size should be pooled or pool allocated
         *
         * @author Julius
         * @date 29.07.2022
         *
         * @returns A bool.
         */
        [[nodiscard]] bool shouldPool(cref<MemoryLayout> layout_, const u64 size_) const noexcept;

        [[nodiscard]] u64 nextPoolSize(const ptr<const MemoryPool> pool_, const u64 requestedSize_) const noexcept;

        void augmentPool(_Inout_ const ptr<MemoryPool> pool_, const u64 size_);

    public:
        [[nodiscard]] AllocationResult allocate(
            cref<MemoryLayout> layout_,
            const u64 size_,
            _Out_ ref<uptr<AllocatedMemory>> dst_
        ) override;

        void free(mref<uptr<AllocatedMemory>> mem_) override;
    };
}

// TODO: Move to any vulkan helper layer
namespace vk {
    class Buffer;
    class Image;
}

namespace hg::engine::gfx {
    class Buffer;
    class Device;
}

namespace hg::engine::gfx::memory {
    [[nodiscard]] AllocationResult allocate(
        const ptr<GlobalPooledAllocator> alloc_,
        cref<sptr<Device>> device_,
        cref<MemoryProperties> props_,
        _Inout_ ref<Buffer> buffer_
    );

    [[nodiscard]] AllocationResult allocate(
        const ptr<GlobalPooledAllocator> alloc_,
        cref<sptr<Device>> device_,
        cref<vk::Buffer> buffer_,
        cref<MemoryProperties> props_,
        ref<uptr<AllocatedMemory>> dst_
    );

    [[nodiscard]] AllocationResult allocate(
        const ptr<GlobalPooledAllocator> alloc_,
        cref<sptr<Device>> device_,
        vk::Image image_,
        cref<MemoryProperties> props_,
        ref<uptr<AllocatedMemory>> dst_
    );
}
