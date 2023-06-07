#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>

#include "__fwd.hpp"

namespace hg::engine::gfx::acc {
    class AccelerationPass {
    public:
        using this_type = AccelerationPass;

    public:
        [[nodiscard]] smr<AccelerationEffect> getEffect() const noexcept;

        [[nodiscard]] cref<Vector<smr<AccelerationStageDerivat>>> getStageDerivates() const noexcept;
    };
}
