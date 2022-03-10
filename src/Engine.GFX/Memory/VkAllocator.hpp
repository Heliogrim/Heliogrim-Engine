#pragma once
#include "Allocator.hpp"

namespace ember::engine::gfx {

    class VkAllocator :
        public Allocator {
    public:
        using this_type = VkAllocator;
        using base_type = Allocator;

    public:
        VkAllocator(cref<sptr<Device>> device_) noexcept;

        ~VkAllocator() noexcept override;

    public:
        [[nodiscard]] ptr<AllocatedMemory> allocate(const u64 size_) override;

        void free(mref<ptr<AllocatedMemory>> memory_) override;

    private:
        sptr<Device> _device;

    public:
        [[nodiscard]] cref<sptr<Device>> device() const noexcept;

    private:
        vk::MemoryPropertyFlags _memoryProperties;
        u32 _memoryTypeBits;

    public:
        [[nodiscard]] static ptr<VkAllocator> makeForBuffer(cref<sptr<Device>> device_, cref<vk::Buffer> buffer_,
            cref<vk::MemoryPropertyFlags> properties_);

        [[nodiscard]] static ptr<VkAllocator> makeForImage(cref<sptr<Device>> device_, cref<vk::Image> image_,
            cref<vk::MemoryPropertyFlags> properties_);
    };
}
