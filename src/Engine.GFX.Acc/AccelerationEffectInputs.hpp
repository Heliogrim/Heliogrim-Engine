#pragma once
#include "Effect/InputLayoutDescription.hpp"

namespace hg::engine::gfx::acc {
    class AccelerationEffectInputs {
    public:
        using this_type = AccelerationEffectInputs;

    private:
        Vector<InputLayoutDescription> _descriptions;

    public:
        void addDescription(cref<InputLayoutDescription> description_);

        [[nodiscard]] cref<Vector<InputLayoutDescription>> getDescriptions() const noexcept;
    };
}
