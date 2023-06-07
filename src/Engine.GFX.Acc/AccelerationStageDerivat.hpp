#pragma once

#include <optional>

#include "AccelerationStage.hpp"

namespace hg::engine::gfx::acc {
    class AccelerationStageDerivat {
    public:
        using this_type = AccelerationStageDerivat;

    public:
        [[nodiscard]] cref<Vector<AccelerationStageInput>> substitutionInputs() const noexcept;

        [[nodiscard]] cref<Vector<AccelerationStageOutput>> substitutionOutputs() const noexcept;

    public:
        [[nodiscard]] std::optional<non_owning_rptr<AccelerationStage>> owner() const noexcept;

        [[nodiscard]] Vector<smr<AccelerationStage>> supersedes() const noexcept;
    };
}
