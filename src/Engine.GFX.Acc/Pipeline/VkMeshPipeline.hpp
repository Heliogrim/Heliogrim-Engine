#pragma once

#include <Engine.Common/Meta/Constexpr.hpp>
#include <Engine.GFX/API/__vkFwd.hpp>
#include <Engine.Reflect/CompileTypeId.hpp>

#include "MeshPipeline.hpp"

namespace hg::engine::gfx::acc {
    class VkMeshPipeline final :
        public MeshPipeline {
    public:
        using this_type = VkMeshPipeline;

    public:
        inline static constexpr type_id typeId { force_constexpr<ctid<this_type>()> };

    public:
        ~VkMeshPipeline() override = default;
    };
}
