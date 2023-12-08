#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.GFX/API/__vkFwd.hpp>

#include "VkDescriptor.hpp"

namespace hg::driver::vk {
    class VkScopedResourceTable;
}

namespace hg::driver::vk {
    class VkResourceTable final {
    public:
        using this_type = VkResourceTable;

    public:
        VkResourceTable() noexcept;

        VkResourceTable(cref<this_type>) = delete;

        VkResourceTable(mref<this_type> other_) noexcept;

        ~VkResourceTable() noexcept;

    private:
        using Holder = smr<void>;
        Vector<Holder> _owned;

    private:
        Vector<VkDescriptorPool> _committedPools;
        Vector<Vector<_::VkDescriptorSet>> _committedSets;

    public:
        void store(mref<VkScopedResourceTable> scopedTable_) noexcept;
    };
}
