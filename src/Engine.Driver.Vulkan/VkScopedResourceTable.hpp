#pragma once
#include <Engine.Accel.Pipeline/__fwd.hpp>
#include <Engine.Accel.Pipeline/Bind/BindLayout.hpp>
#include <Engine.Common/Collection/InlineAutoArray.hpp>
#include <Engine.GFX.Render.Command/Resource/ResourceTable.hpp>
#include <Engine.GFX/API/__vkFwd.hpp>

#include "VkDescriptor.hpp"

namespace hg::driver::vk {
    class VkScopedResourceTable :
        public engine::render::ResourceTable {
    public:
        friend class VkResourceTable;

    public:
        using this_type = VkScopedResourceTable;

    public:
        VkScopedResourceTable() noexcept = default;

        ~VkScopedResourceTable() noexcept = default;

    private:
        nmpt<const engine::accel::AccelerationPipeline> _activePipeline;

    public:
        [[nodiscard]] nmpt<const engine::accel::AccelerationPipeline> getActivePipeline() const noexcept;

        void replaceActivePipeline(nmpt<const engine::accel::AccelerationPipeline> pipeline_) noexcept;

    public:
        using Holder = smr<void>;

    private:
        Vector<Holder> _mayOwned;
        Vector<Holder> _owned;

    public:
        void attach(mref<Holder> obj_);

        void detach(cref<Holder> obj_);

    public:
        [[nodiscard]] ref<this_type> replaceWith(cref<ResourceTable> table_) noexcept;

        [[nodiscard]] ref<this_type> replaceWith(mref<ResourceTable> table_) noexcept;

    private:
        InlineAutoArray<VkDescriptorPool> _commitPools;
        InlineAutoArray<Vector<_::VkDescriptorSet>> _committedSets;

    private:
        [[nodiscard]] Vector<VkDescriptorPoolSize> nextAllocSizes() const noexcept;

        [[nodiscard]] bool allocateAndCommit(
            _In_ mref<Vector<VkDescriptorPoolSize>> allocationLayout_,
            _In_ cref<engine::accel::BindLayout> commitLayout_,
            _Out_ ref<Vector<_::VkDescriptorSet>> descriptorSets_
        ) noexcept;

        void updateSets(
            _In_ cref<engine::accel::BindLayout> layout_,
            _Inout_ ref<Vector<_::VkDescriptorSet>> descriptorSets_
        );

    public:
        /**
         * @details
         *
         * @returns true if the binding layout was exhaustively committed.
         */
        [[nodiscard]] bool commit(
            _In_ cref<engine::accel::BindLayout> layout_,
            _Out_ ref<Vector<_::VkDescriptorSet>> descriptorSets_
        ) noexcept;
    };
}
