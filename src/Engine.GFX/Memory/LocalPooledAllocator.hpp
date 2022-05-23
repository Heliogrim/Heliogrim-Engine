#pragma once

#include <Engine.Common/Wrapper.hpp>

#include "__fwd.hpp"
#include "MemoryCache.hpp"

namespace ember::engine::gfx::memory {

    class LocalPooledAllocator {
    public:
        using this_type = LocalPooledAllocator;

    public:
        LocalPooledAllocator(const ptr<GlobalPooledAllocator> global_);

        ~LocalPooledAllocator();

    public:
        void tidy();

    private:
        ptr<GlobalPooledAllocator> _global;

    private:
        MemoryCache _cache;

    public:
        [[nodiscard]] AllocationResult allocate(cref<MemoryLayout> layout_, const u64 size_,
            _Out_ ref<ptr<AllocatedMemory>> dst_);

        void free(mref<ptr<AllocatedMemory>> mem_);
    };
}

// TODO: Move to any vulkan helper layer
namespace vk {
    class Buffer;
    class Image;
}

namespace ember::engine::gfx {
    class Device;
}

namespace ember::engine::gfx::memory {
    [[nodiscard]] AllocationResult allocate(const ptr<LocalPooledAllocator> alloc_, cref<sptr<Device>> device_,
        cref<vk::Buffer> buffer_, cref<MemoryProperties> props_, ref<ptr<AllocatedMemory>> dst_);

    [[nodiscard]] AllocationResult allocate(const ptr<LocalPooledAllocator> alloc_, cref<sptr<Device>> device_,
        vk::Image image_, cref<MemoryProperties> props_, ref<ptr<AllocatedMemory>> dst_);
}
