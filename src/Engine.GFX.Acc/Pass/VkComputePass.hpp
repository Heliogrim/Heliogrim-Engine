#pragma once

#include <Engine.Common/Meta/Constexpr.hpp>
#include <Engine.GFX/API/__vkFwd.hpp>
#include <Engine.Reflect/CompileTypeId.hpp>

#include "ComputePass.hpp"

namespace hg::engine::gfx::acc {
    class VkComputePass final :
        public InheritMeta<VkComputePass, ComputePass> {
    public:
        using this_type = VkComputePass;

    public:
        inline static constexpr type_id typeId { force_constexpr<ctid<this_type>()> };

    public:
        ~VkComputePass() override = default;

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
