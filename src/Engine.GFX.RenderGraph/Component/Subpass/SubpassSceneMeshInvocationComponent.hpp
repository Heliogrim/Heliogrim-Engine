#pragma once

#include <Engine.Common/Meta/Constexpr.hpp>
#include <Engine.Reflect/CompileTypeId.hpp>

#include "SubpassInvocationComponent.hpp"

namespace hg::engine::gfx::render::graph {
    class SubpassSceneMeshInvocationComponent final :
        public InheritMeta<SubpassSceneMeshInvocationComponent, SubpassInvocationComponent> {
    public:
        using this_type = SubpassSceneMeshInvocationComponent;

    public:
        inline static constexpr type_id typeId { force_constexpr<ctid<this_type>()> };

    public:
        SubpassSceneMeshInvocationComponent() noexcept = default;

        ~SubpassSceneMeshInvocationComponent() noexcept override = default;
    };
}
