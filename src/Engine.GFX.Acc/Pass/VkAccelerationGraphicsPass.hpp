#pragma once

#include <Engine.Common/Meta/Constexpr.hpp>
#include <Engine.GFX/API/__vkFwd.hpp>
#include <Engine.Reflect/CompileTypeId.hpp>

#include "AccelerationGraphicsPass.hpp"

namespace hg::engine::gfx::acc {
    class VkAccelerationGraphicsPass final :
        public AccelerationGraphicsPass {
    public:
        using this_type = VkAccelerationGraphicsPass;

    public:
        inline static constexpr type_id typeId { force_constexpr<ctid<this_type>()> };

    public:
        VkAccelerationGraphicsPass(mref<smr<AccelerationEffect>> effect_);

        ~VkAccelerationGraphicsPass() override = default;

    public:
        void setStageDerivat(const size_t idx_, mref<smr<AccelerationStageDerivat>> derivat_);

    private:
        _::VkGraphicsPipelineLayout _vkPipeLayout;
        _::VkGraphicsPipeline _vkPipe;

    public:
        void setVkPipeLayout(mref<_::VkGraphicsPipelineLayout> vkPipeLayout_);

        void setVkPipe(mref<_::VkGraphicsPipeline> vkPipe_);
    };
}
