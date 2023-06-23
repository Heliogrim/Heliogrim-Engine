#pragma once

#include <Engine.Common/Meta/Constexpr.hpp>
#include <Engine.GFX/API/__vkFwd.hpp>
#include <Engine.Reflect/CompileTypeId.hpp>

#include "AccelerationMeshPass.hpp"

namespace hg::engine::gfx::acc {
    class VkAccelerationMeshPass final :
        public AccelerationMeshPass {
    public:
        using this_type = VkAccelerationMeshPass;

    public:
        inline static constexpr type_id typeId { force_constexpr<ctid<this_type>()> };

    public:
        ~VkAccelerationMeshPass() override = default;
    };
}
