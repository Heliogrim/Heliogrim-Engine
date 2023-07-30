#pragma once

#include <span>
#include <Engine.Common/Collection/Set.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Meta/Constexpr.hpp>
#include <Engine.GFX.Acc/AccelerationEffect.hpp>
#include <Engine.Reflect/CompileTypeId.hpp>

#include "CompileSubpassAccelComponent.hpp"

namespace hg::engine::gfx::render::graph {
    class CompileSubpassMultiAccelComponent final :
        public InheritMeta<CompileSubpassMultiAccelComponent, CompileSubpassAccelComponent> {
    public:
        using this_type = CompileSubpassMultiAccelComponent;

    public:
        inline static constexpr type_id typeId { force_constexpr<ctid<this_type>()> };

    public:
        CompileSubpassMultiAccelComponent() noexcept = default;

        ~CompileSubpassMultiAccelComponent() noexcept override;

    private:
        Vector<smr<const acc::AccelerationEffect>> _accelerations;

    public:
        [[nodiscard]] _STD span<const smr<const acc::AccelerationEffect>> getAccelEffects() const noexcept;

        [[nodiscard]] bool hasAccelEffect(mref<smr<const acc::AccelerationEffect>> effect_) const noexcept;

        void addAccelEffect(mref<smr<const acc::AccelerationEffect>> effect_);

        void removeAccelEffect(mref<smr<const acc::AccelerationEffect>> effect_);
    };
}
