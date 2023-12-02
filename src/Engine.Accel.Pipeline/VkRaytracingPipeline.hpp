#pragma once

#include <Engine.Common/Meta/Constexpr.hpp>
#include <Engine.GFX/API/__vkFwd.hpp>
#include <Engine.Reflect/CompileTypeId.hpp>

#include "RaytracingPipeline.hpp"

namespace hg::engine::accel {
    class VkRaytracingPipeline final :
        public RaytracingPipeline {
    public:
        using this_type = VkRaytracingPipeline;

    public:
        inline static constexpr type_id typeId { force_constexpr<ctid<this_type>()> };

    public:
        ~VkRaytracingPipeline() override = default;
    };
}
