#pragma once

#include <Engine.Common/Meta/Constexpr.hpp>
#include <Engine.GFX/API/__vkFwd.hpp>
#include <Engine.Reflect/CompileTypeId.hpp>

#include "ComputePipeline.hpp"

namespace hg::engine::accel {
    class VkComputePipeline final :
        public InheritMeta<VkComputePipeline, ComputePipeline> {
    public:
        using this_type = VkComputePipeline;

    public:
        inline static constexpr type_id typeId { force_constexpr<ctid<this_type>()> };

    public:
        ~VkComputePipeline() override = default;

    private:
        void tidy();

    private:
    public:
        _::VkComputePipelineLayout _vkPipeLayout;
        _::VkComputePipeline _vkPipe;

    public:
        void setVkPipeLayout(mref<_::VkComputePipelineLayout> vkPipeLayout_);

        void setVkPipe(mref<_::VkComputePipeline> vkPipe_);

        // Warning: Temporary
    public:
        Vector<_::VkDescriptorSetLayout> _vkDescLayouts;
    };
}
