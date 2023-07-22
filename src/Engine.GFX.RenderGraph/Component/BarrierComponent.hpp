#pragma once
#include <Engine.Common/String.hpp>
#include <Engine.Common/Meta/Constexpr.hpp>
#include <Engine.Reflect/CompileTypeId.hpp>

#include "Component.hpp"

namespace hg::engine::gfx::render::graph {
    class BarrierComponent final :
        public InheritMeta<BarrierComponent, Component> {
    public:
        using this_type = BarrierComponent;

    public:
        inline static constexpr type_id typeId { force_constexpr<ctid<this_type>()> };

    public:
        BarrierComponent() noexcept = default;

        BarrierComponent(mref<string> barrierName_) noexcept;

        ~BarrierComponent() noexcept override = default;

    private:
        string _name;

    public:
        [[nodiscard]] string_view getBarrierName() const noexcept;

        void setBarrierName(string_view barrierName_);
    };
}
