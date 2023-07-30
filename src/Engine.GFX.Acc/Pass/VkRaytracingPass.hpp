#pragma once

#include <Engine.Common/Meta/Constexpr.hpp>
#include <Engine.GFX/API/__vkFwd.hpp>
#include <Engine.Reflect/CompileTypeId.hpp>

#include "RaytracingPass.hpp"

namespace hg::engine::gfx::acc {
    class VkRaytracingPass final :
        public RaytracingPass {
    public:
        using this_type = VkRaytracingPass;

    public:
        inline static constexpr type_id typeId { force_constexpr<ctid<this_type>()> };

    public:
        ~VkRaytracingPass() override = default;
    };
}
