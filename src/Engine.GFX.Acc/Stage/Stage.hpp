#pragma once
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.GFX.Acc.Lang/Intermediate.hpp>

#include "../__fwd.hpp"
#include "StageFlags.hpp"
#include "StageModule.hpp"
#include "StageInput.hpp"
#include "StageOutput.hpp"

namespace hg::engine::gfx::acc {
    class Stage {
    public:
        using this_type = Stage;

    public:
        Stage() noexcept = default;

        Stage(
            mref<StageFlagBits> flagBits_,
            mref<Vector<StageInput>> stageInputs_,
            mref<Vector<StageOutput>> stageOutputs_
        );

        virtual ~Stage();

    protected:
        StageFlagBits _flagBits;

    public:
        [[nodiscard]] StageFlagBits getFlagBits() const noexcept;

    protected:
        Vector<StageInput> _stageInputs;
        Vector<StageOutput> _stageOutputs;

    public:
        [[nodiscard]] cref<Vector<StageInput>> getStageInputs() const noexcept;

        [[nodiscard]] cref<Vector<StageOutput>> getStageOutputs() const noexcept;

    protected:
        smr<lang::Intermediate> _intermediate;

    public:
        [[nodiscard]] smr<lang::Intermediate> getIntermediate() const noexcept;

        void setIntermediate(mref<smr<lang::Intermediate>> intermediate_);
    };
}
