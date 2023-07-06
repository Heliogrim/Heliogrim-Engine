#pragma once
#include <Engine.Common/String.hpp>

#include "Component.hpp"

namespace hg::engine::gfx::render::graph {
    class BarrierComponent final :
        public Component {
    public:
        using this_type = BarrierComponent;

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
