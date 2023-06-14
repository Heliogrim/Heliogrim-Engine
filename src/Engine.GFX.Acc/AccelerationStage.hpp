#pragma once
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>

#include "__fwd.hpp"
#include "AccelerationStageFlags.hpp"
#include "AccelerationStageModule.hpp"
#include "AccelerationStageInput.hpp"
#include "AccelerationStageOutput.hpp"

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

        virtual ~AccelerationStage();

    private:
        smr<AccelerationStageModule> _stageModule;

    public:
        [[nodiscard]] smr<AccelerationStageModule> getStageModule() const noexcept;

    private:
        AccelerationStageFlags _flags;

    public:
        [[nodiscard]] AccelerationStageFlags getFlags() const noexcept;

    private:
        Vector<AccelerationStageInput> _stageInputs;
        Vector<AccelerationStageOutput> _stageOutputs;

    public:
        [[nodiscard]] cref<Vector<AccelerationStageInput>> getStageInputs() const noexcept;

        [[nodiscard]] cref<Vector<AccelerationStageOutput>> getStageOutputs() const noexcept;
    };
}
