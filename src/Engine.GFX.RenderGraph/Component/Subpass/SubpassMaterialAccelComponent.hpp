#pragma once

#include <span>
#include <Engine.Common/Collection/DenseMap.hpp>
#include <Engine.Common/Collection/Set.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Meta/Constexpr.hpp>
#include <Engine.GFX.Acc.Compile/Profile/EffectProfile.hpp>
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
        // Material -> List < Accel Pass >
        // `?` * Profile -> List < Accel Pass >
        // => Map < Material ~> Map < Profile ~> List < Accel Pass > > >
        // => Map < Profile ~> Map < Material ~> List < Accel Pass > > >
        // O ( N_p * N_m * k_pm )

        /**
         * @details We expect to have less effective profiles for the compiling process than applied materials.
         *  Therefore hoisting the hyper-mapping should be more efficient, cause the underlying
         *  material mapping might change rapidly.
         */
        DenseMap<smr<const acc::EffectProfile>, DenseMap<smr<MaterialResource>, Vector<smr<const
            acc::AccelerationPass>>>> _accelPasses;

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
