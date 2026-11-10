#pragma once
#include "Allocator.hpp"
#include "../Device/Device.hpp"

namespace hg::engine::gfx::memory {
    class VkAllocator final :
        public Allocator {
    public:
        using this_type = VkAllocator;

    public:
        VkAllocator(cref<sptr<Device>> device_) noexcept;

        ~VkAllocator() noexcept override;

    private:
        sptr<Device> _device;

    public:
        [[nodiscard]] cref<sptr<Device>> device() const noexcept;

    public:
        [[nodiscard]] AllocationResult allocate(
            cref<MemoryLayout> layout_,
            const u64 size_,
            ref<uptr<AllocatedMemory>> dst_
        ) override;

        void free(mref<uptr<AllocatedMemory>> mem_) override;
    };
}
