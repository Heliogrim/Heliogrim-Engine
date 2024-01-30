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

    public:
        ref<this_type> operator=(cref<this_type>) = delete;

        ref<this_type> operator=(mref<this_type> other_) noexcept = default;

    private:
        using Holder = smr<void>;
        Vector<Holder> _owned;

    private:
        Vector<VkDescriptorPool> _committedPools;
        Vector<Vector<_::VkDescriptorSet>> _committedSets;

    public:
        [[nodiscard]] uptr<VkScopedResourceTable> makeScoped() noexcept;

        void merge(mref<uptr<VkScopedResourceTable>> scopedTable_) noexcept;

        void store(mref<VkScopedResourceTable> scopedTable_) noexcept;

        void reset() noexcept;
    };
}
