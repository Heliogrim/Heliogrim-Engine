#pragma once
#include "Layout/InputLayoutDescription.hpp"

namespace hg::engine::gfx::acc {
    class AccelerationInputLayout {
    public:
        using this_type = AccelerationInputLayout;

    private:
        Vector<InputLayoutDescription> _descriptions;

    public:
        void addDescription(cref<InputLayoutDescription> description_);

        [[nodiscard]] cref<Vector<InputLayoutDescription>> getDescriptions() const noexcept;
    };
}
