#pragma once

#include <Engine.Common/Meta/Constexpr.hpp>
#include <Engine.GFX/API/__vkFwd.hpp>
#include <Engine.Reflect/CompileTypeId.hpp>

#include "GraphicsPipeline.hpp"

namespace hg::engine::gfx::acc {
    class VkGraphicsPipeline final :
        public InheritMeta<VkGraphicsPipeline, GraphicsPipeline> {
    public:
        using this_type = VkGraphicsPipeline;

    public:
        inline static constexpr type_id typeId { force_constexpr<ctid<this_type>()> };

    public:
        VkGraphicsPipeline(mref<smr<const AccelerationEffect>> effect_);

        ~VkGraphicsPipeline() override;

    private:
        void tidy();

    public:
        void setStageDerivat(const size_t idx_, mref<smr<StageDerivat>> derivat_);

    private:
    public:
        _::VkGraphicsPipelineLayout _vkPipeLayout;
        _::VkGraphicsPipeline _vkPipe;

    public:
        void setVkPipeLayout(mref<_::VkGraphicsPipelineLayout> vkPipeLayout_);

        void setVkPipe(mref<_::VkGraphicsPipeline> vkPipe_);

        // Warning: Temporary
    public:
        Vector<_::VkDescriptorSetLayout> _vkDescLayouts;
    };
}
