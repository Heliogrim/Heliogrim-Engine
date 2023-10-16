#pragma once

#include <Engine.Common/Meta/Constexpr.hpp>
#include <Engine.Reflect/CompileTypeId.hpp>

#include "SubpassInvocationComponent.hpp"

namespace hg::engine::gfx::render::graph {
    class SubpassComputeInvocationComponent final :
        public InheritMeta<SubpassComputeInvocationComponent, SubpassInvocationComponent> {
    public:
        using this_type = SubpassComputeInvocationComponent;

    public:
        SubpassComputeInvocationComponent() noexcept = default;

        ~SubpassComputeInvocationComponent() noexcept override = default;
    };
}
