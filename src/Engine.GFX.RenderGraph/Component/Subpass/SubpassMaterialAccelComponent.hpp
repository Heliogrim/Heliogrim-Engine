#pragma once

#include <span>
#include <Engine.Common/Collection/DenseMap.hpp>
#include <Engine.Common/Collection/Set.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Meta/Constexpr.hpp>
#include <Engine.GFX.Loader/Material/MaterialResource.hpp>
#include <Engine.Reflect/CompileTypeId.hpp>

#include "SubpassAccelComponent.hpp"

namespace hg::engine::gfx::render::graph {
    class SubpassMaterialAccelComponent final :
        public InheritMeta<SubpassMaterialAccelComponent, SubpassAccelComponent> {
    public:
        using this_type = SubpassMaterialAccelComponent;

    public:
        inline static constexpr type_id typeId { force_constexpr<ctid<this_type>()> };

    public:
        SubpassMaterialAccelComponent() noexcept = default;

        ~SubpassMaterialAccelComponent() noexcept override;

    private:
        DenseMap<smr<MaterialResource>, Vector<smr<const acc::AccelerationPass>>> _accelPasses;

    public:
        [[nodiscard]] Vector<smr<acc::Symbol>> aggregateImportedSymbols() const override;

        [[nodiscard]] Vector<smr<acc::Symbol>> aggregateExportedSymbols() const override;

    private:
        Vector<smr<MaterialResource>> _materials;

    public:
        [[nodiscard]] _STD span<const smr<MaterialResource>> getMaterials() const noexcept;

        [[nodiscard]] bool hasMaterial(mref<smr<MaterialResource>> material_) const noexcept;

        void addMaterial(mref<smr<MaterialResource>> material_);

        void removeMaterial(mref<smr<MaterialResource>> material_);
    };
}
