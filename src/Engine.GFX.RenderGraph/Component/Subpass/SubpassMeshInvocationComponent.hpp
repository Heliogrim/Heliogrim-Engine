#pragma once

#include <Engine.Common/Meta/Constexpr.hpp>
#include <Engine.Reflect/CompileTypeId.hpp>

#include "SubpassInvocationComponent.hpp"
#include "../../Relation/MeshDescription.hpp"

namespace hg::engine::gfx::render::graph {
    class SubpassMeshInvocationComponent final :
        public InheritMeta<SubpassMeshInvocationComponent, SubpassInvocationComponent> {
    public:
        using this_type = SubpassMeshInvocationComponent;

    public:
        SubpassMeshInvocationComponent() noexcept = default;

        ~SubpassMeshInvocationComponent() noexcept override = default;

    private:
        smr<MeshDescription> _meshDataLayout;

    public:
        [[nodiscard]] decltype(_meshDataLayout) getMeshDataLayout() const noexcept;
    };
}
