#pragma once

#include <Engine.Common/Meta/Constexpr.hpp>
#include <Engine.GFX/API/__vkFwd.hpp>
#include <Engine.Reflect/CompileTypeId.hpp>

#include "AccelerationRaytracingPass.hpp"

namespace hg::engine::gfx::acc {
    class VkAccelerationRaytracingPass final :
        public AccelerationRaytracingPass {
    public:
        using this_type = VkAccelerationRaytracingPass;

    public:
        inline static constexpr type_id typeId { force_constexpr<ctid<this_type>()> };

    public:
        ~VkAccelerationRaytracingPass() override = default;
    };
}
