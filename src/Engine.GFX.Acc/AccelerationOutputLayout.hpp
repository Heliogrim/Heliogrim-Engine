#pragma once
#include "Layout/OutputLayoutDescription.hpp"

namespace hg::engine::gfx::acc {
    class AccelerationOutputLayout {
    public:
        using this_type = AccelerationOutputLayout;

    private:
        Vector<OutputLayoutDescription> _descriptions;

    public:
        void addDescription(cref<OutputLayoutDescription> description_);

        [[nodiscard]] cref<Vector<OutputLayoutDescription>> getDescriptions() const noexcept;
    };
}
