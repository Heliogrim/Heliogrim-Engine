#pragma once
#include "Layout/BindingLayoutDescription.hpp"

namespace hg::engine::gfx::acc {
    class AccelerationBindingLayout {
    public:
        using this_type = AccelerationBindingLayout;

    private:
        Vector<BindingLayoutDescription> _descriptions;

    public:
        void addDescription(cref<BindingLayoutDescription> description_);

        [[nodiscard]] cref<Vector<BindingLayoutDescription>> getDescriptions() const noexcept;
    };
}
