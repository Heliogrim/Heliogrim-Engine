#pragma once

#include <span>
#include <Engine.Common/Collection/DenseMap.hpp>
#include <Engine.Common/Meta/Constexpr.hpp>
#include <Engine.GFX.Acc/AccelerationEffect.hpp>
#include <Engine.GFX.Acc.Compile/Profile/EffectProfile.hpp>
#include <Engine.Reflect/CompileTypeId.hpp>

#include "SubpassAccelComponent.hpp"

namespace hg::engine::gfx::render::graph {
    class SubpassSingleAccelComponent final :
        public InheritMeta<SubpassSingleAccelComponent, SubpassAccelComponent> {
    public:
        using this_type = SubpassSingleAccelComponent;

    public:
        inline static constexpr type_id typeId { force_constexpr<ctid<this_type>()> };

    public:
        SubpassSingleAccelComponent() noexcept = default;

        ~SubpassSingleAccelComponent() noexcept override;

    private:
        DenseMap<smr<const acc::EffectProfile>, smr<const acc::AccelerationPass>> _accelPass;

    public:
        [[nodiscard]] Vector<smr<acc::Symbol>> aggregateImportedSymbols() const override;

        [[nodiscard]] Vector<smr<acc::Symbol>> aggregateExportedSymbols() const override;

    private:
        smr<const acc::AccelerationEffect> _acceleration;

    public:
        [[nodiscard]] smr<const acc::AccelerationEffect> getAccelEffect() const noexcept;

        void setAccelEffect(mref<smr<const acc::AccelerationEffect>> effect_);

        void clearAccelEffect(mref<smr<const acc::AccelerationEffect>> effect_ = nullptr);
    };
}
