#pragma once

#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.GFX.Acc/AccelerationPass.hpp>
#include "../Token/ScopedTokenStorage.hpp"

namespace hg::engine::gfx::acc {
    class StageComposer {
    public:
        using this_type = StageComposer;

    public:
        StageComposer() noexcept = default;

        ~StageComposer() noexcept = default;

    public:
        [[nodiscard]] Vector<smr<AccelerationStageDerivat>> compose(
            cref<smr<AccelerationPass>> targetPass_,
            cref<ScopedTokenStorage> tokens_
        ) const;
    };
}
