#pragma once
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>

#include "__fwd.hpp"
#include "AccelerationStageFlags.hpp"

namespace hg::engine::gfx::acc {
    class AccelerationStage {
    public:
        using this_type = AccelerationStage;

    public:
        AccelerationStage() noexcept = default;

        AccelerationStage(
            mref<smr<AccelerationStageModule>> stageModule_,
            mref<AccelerationStageFlags> flags_,
            mref<Vector<AccelerationStageInput>> stageInputs_,
            mref<Vector<AccelerationStageOutput>> stageOutputs_
        );

        ~AccelerationStage();

    public:
        [[nodiscard]] smr<AccelerationStageModule> getStageModule() const noexcept;

        [[nodiscard]] AccelerationStageFlags getFlags() const noexcept;

        [[nodiscard]] cref<Vector<AccelerationStageInput>> getStageInputs() const noexcept;

        [[nodiscard]] cref<Vector<AccelerationStageOutput>> getStageOutputs() const noexcept;
    };
}
