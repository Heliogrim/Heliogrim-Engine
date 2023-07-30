#pragma once

#include <span>
#include <Engine.Common/Collection/Set.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Meta/Constexpr.hpp>
#include <Engine.GFX.Loader/Material/MaterialResource.hpp>

#include "CompileSubpassAccelComponent.hpp"

namespace hg::engine::gfx::render::graph {
    class CompileSubpassMaterialAccelComponent final :
        public InheritMeta<CompileSubpassMaterialAccelComponent, CompileSubpassAccelComponent> {
    public:
        using this_type = CompileSubpassMaterialAccelComponent;

    public:
        CompileSubpassMaterialAccelComponent() noexcept = default;

        ~CompileSubpassMaterialAccelComponent() noexcept override;

    private:
        Vector<smr<MaterialResource>> _materials;

    public:
        [[nodiscard]] _STD span<const smr<MaterialResource>> getMaterials() const noexcept;

        [[nodiscard]] bool hasMaterial(mref<smr<MaterialResource>> material_) const noexcept;

        void addMaterial(mref<smr<MaterialResource>> material_);

        void removeMaterial(mref<smr<MaterialResource>> material_);
    };
}
