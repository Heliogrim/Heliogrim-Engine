#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>

#include "__fwd.hpp"

namespace hg::engine::gfx::acc {
    class AccelerationPass {
    public:
        using this_type = AccelerationPass;

    public:
        AccelerationPass() noexcept;

        AccelerationPass(
            mref<smr<AccelerationEffect>> effect_,
            mref<Vector<smr<AccelerationStageDerivat>>> stages_
        ) noexcept;

        AccelerationPass(mref<this_type> other_) noexcept = default;

        AccelerationPass(cref<this_type> other_) = default;

        virtual ~AccelerationPass();

    private:
        smr<AccelerationEffect> _effect;
        Vector<smr<AccelerationStageDerivat>> _stages;

    public:
        [[nodiscard]] smr<AccelerationEffect> getEffect() const noexcept;

        [[nodiscard]] cref<Vector<smr<AccelerationStageDerivat>>> getStageDerivates() const noexcept;
    };
}
