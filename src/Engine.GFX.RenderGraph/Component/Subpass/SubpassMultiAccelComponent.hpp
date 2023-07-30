#pragma once

#include <span>
#include <Engine.Common/Collection/Set.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Meta/Constexpr.hpp>
#include <Engine.GFX.Acc/AccelerationEffect.hpp>
#include <Engine.Reflect/CompileTypeId.hpp>

#include "SubpassAccelComponent.hpp"

namespace hg::engine::gfx::render::graph {
    class SubpassMultiAccelComponent final :
        public InheritMeta<SubpassMultiAccelComponent, SubpassAccelComponent> {
    public:
        using this_type = SubpassMultiAccelComponent;

    public:
        inline static constexpr type_id typeId { force_constexpr<ctid<this_type>()> };

    public:
        SubpassMultiAccelComponent() noexcept = default;

        ~SubpassMultiAccelComponent() noexcept override;

    private:
        Vector<smr<const acc::AccelerationPass>> _accelPasses;

    public:
        [[nodiscard]] Vector<smr<acc::Symbol>> aggregateImportedSymbols() const override;

        [[nodiscard]] Vector<smr<acc::Symbol>> aggregateExportedSymbols() const override;

    private:
        Vector<smr<const acc::AccelerationEffect>> _accelerations;

    public:
        [[nodiscard]] _STD span<const smr<const acc::AccelerationEffect>> getAccelEffects() const noexcept;

        [[nodiscard]] bool hasAccelEffect(mref<smr<const acc::AccelerationEffect>> effect_) const noexcept;

        void addAccelEffect(mref<smr<const acc::AccelerationEffect>> effect_);

        void removeAccelEffect(mref<smr<const acc::AccelerationEffect>> effect_);
    };
}
