#pragma once

#include <Engine.Common/Meta/Constexpr.hpp>
#include <Engine.Reflect/CompileTypeId.hpp>

#include "ComputePass.hpp"

namespace hg::engine::gfx::acc {
    class VkComputePass final :
        public ComputePass {
    public:
        using this_type = VkComputePass;

    public:
        inline static constexpr type_id typeId { force_constexpr<ctid<this_type>()> };

    public:
        ~VkComputePass() override = default;
    };
}
