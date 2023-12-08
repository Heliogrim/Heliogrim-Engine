#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.GFX/vkinc.hpp>
#include <Engine.Common/Math/Vector.hpp>

typedef struct VkDescriptorPool_T* VkDescriptorPool;

namespace hg::driver::vk {
    struct VkDescriptorPoolSize {
        ::vk::DescriptorType type;
        u32 capacity;
    };

    class VkDescriptorPool {
    public:
        using this_type = VkDescriptorPool;

    public:
        VkDescriptorPool() noexcept;

        VkDescriptorPool(cref<this_type>) = delete;

        VkDescriptorPool(mref<this_type> other_) noexcept;

        ~VkDescriptorPool() noexcept;

    public:
        Vector<VkDescriptorPoolSize> pooled;
        ::VkDescriptorPool vkPool;
    };
}
