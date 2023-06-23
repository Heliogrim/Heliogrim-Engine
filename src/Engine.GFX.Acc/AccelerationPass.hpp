#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.Reflect/HeliogrimReflect.hpp>

#include "__fwd.hpp"

namespace hg::engine::gfx::acc {
    class AccelerationPass :
        public HeliogrimObject {
    public:
        using this_type = AccelerationPass;

    protected:
        AccelerationPass(mref<smr<AccelerationEffect>> effect_) noexcept;

    public:
        AccelerationPass(mref<this_type> other_) noexcept = delete;

        AccelerationPass(cref<this_type> other_) = delete;

        virtual ~AccelerationPass();

    protected:
        smr<AccelerationEffect> _effect;
        Vector<smr<AccelerationStageDerivat>> _stages;

    public:
        [[nodiscard]] smr<AccelerationEffect> getEffect() const noexcept;

        [[nodiscard]] cref<Vector<smr<AccelerationStageDerivat>>> getStageDerivates() const noexcept;
    };
}
