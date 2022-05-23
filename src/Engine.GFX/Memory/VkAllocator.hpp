#pragma once
#include "Allocator.hpp"

namespace ember::engine::gfx::memory {

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
        [[nodiscard]] AllocationResult allocate(cref<MemoryLayout> layout_, const u64 size_,
            ref<ptr<AllocatedMemory>> dst_) override;

        void free(mref<ptr<AllocatedMemory>> mem_) override;
    };
}
