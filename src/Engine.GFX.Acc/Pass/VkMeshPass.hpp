#pragma once

#include <Engine.Common/Meta/Constexpr.hpp>
#include <Engine.GFX/API/__vkFwd.hpp>
#include <Engine.Reflect/CompileTypeId.hpp>

#include "MeshPass.hpp"

namespace hg::engine::gfx::acc {
    class VkMeshPass final :
        public MeshPass {
    public:
        using this_type = VkMeshPass;

    public:
        inline static constexpr type_id typeId { force_constexpr<ctid<this_type>()> };

    public:
        ~VkMeshPass() override = default;
    };
}
