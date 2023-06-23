#pragma once
#include "Effect/BindingLayoutDescription.hpp"

namespace hg::engine::gfx::acc {
    class AccelerationEffectBindings {
    public:
        using this_type = AccelerationEffectBindings;

    private:
        Vector<BindingLayoutDescription> _descriptions;

    public:
        void addDescription(cref<BindingLayoutDescription> description_);

        [[nodiscard]] cref<Vector<BindingLayoutDescription>> getDescriptions() const noexcept;
    };
}
