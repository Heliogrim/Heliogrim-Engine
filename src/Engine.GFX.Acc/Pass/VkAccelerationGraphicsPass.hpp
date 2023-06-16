#pragma once

#include <Engine.GFX/API/__vkFwd.hpp>

#include "AccelerationGraphicsPass.hpp"

namespace hg::engine::gfx::acc {
    class VkAccelerationGraphicsPass :
        public AccelerationGraphicsPass {
    public:
        using this_type = VkAccelerationGraphicsPass;

    public:
        VkAccelerationGraphicsPass(mref<smr<AccelerationEffect>> effect_);

        ~VkAccelerationGraphicsPass() override = default;

    public:
        void setStageDerivat(const size_t idx_, mref<smr<AccelerationStageDerivat>> derivat_);

    public:
        void setVkPipe(mref<_::VkGraphicsPipeline> vkPipe_);
    };
}
