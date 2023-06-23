#pragma once

#include <Engine.Common/Meta/Constexpr.hpp>
#include <Engine.Reflect/CompileTypeId.hpp>

#include "AccelerationComputePass.hpp"

namespace hg::engine::gfx::acc {
    class VkAccelerationComputePass final :
        public AccelerationComputePass {
    public:
        using this_type = VkAccelerationComputePass;

    public:
        inline static constexpr type_id typeId { force_constexpr<ctid<this_type>()> };

    public:
        ~VkAccelerationComputePass() override = default;
    };
}
