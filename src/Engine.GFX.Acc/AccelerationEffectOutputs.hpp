#pragma once
#include "Effect/OutputLayoutDescription.hpp"

namespace hg::engine::gfx::acc {
    class AccelerationEffectOutputs {
    public:
        using this_type = AccelerationEffectOutputs;

    private:
        Vector<OutputLayoutDescription> _descriptions;

    public:
        void addDescription(cref<OutputLayoutDescription> description_);

        [[nodiscard]] cref<Vector<OutputLayoutDescription>> getDescriptions() const noexcept;
    };
}
