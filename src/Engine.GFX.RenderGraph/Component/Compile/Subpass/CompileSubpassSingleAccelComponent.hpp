#pragma once

#include <span>
#include <Engine.Common/Meta/Constexpr.hpp>
#include <Engine.GFX.Acc/AccelerationEffect.hpp>
#include <Engine.Reflect/CompileTypeId.hpp>

#include "CompileSubpassAccelComponent.hpp"

namespace hg::engine::gfx::render::graph {
    class CompileSubpassSingleAccelComponent final :
        public InheritMeta<CompileSubpassSingleAccelComponent, CompileSubpassAccelComponent> {
    public:
        using this_type = CompileSubpassSingleAccelComponent;

    public:
        inline static constexpr type_id typeId { force_constexpr<ctid<this_type>()> };

    public:
        CompileSubpassSingleAccelComponent() noexcept = default;

        ~CompileSubpassSingleAccelComponent() noexcept override;

    private:
        smr<const acc::AccelerationEffect> _acceleration;

    public:
        [[nodiscard]] smr<const acc::AccelerationEffect> getAccelEffect() const noexcept;

        void setAccelEffect(mref<smr<const acc::AccelerationEffect>> effect_);

        void clearAccelEffect(mref<smr<const acc::AccelerationEffect>> effect_ = nullptr);
    };
}
