#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

#include "AccelerationStage.hpp"

namespace hg::engine::gfx::acc {
    class AccelerationStageDerivat :
        public AccelerationStage {
    public:
        using this_type = AccelerationStageDerivat;

    public:
        AccelerationStageDerivat() noexcept;

        AccelerationStageDerivat(
            mref<nmpt<AccelerationStage>> owner_,
            mref<Vector<smr<AccelerationStage>>> supersedes_,
            mref<smr<AccelerationStageModule>> stageModule_,
            mref<AccelerationStageFlagBits> flagBits_,
            mref<Vector<AccelerationStageInput>> stageInputs_,
            mref<Vector<AccelerationStageOutput>> stageOutputs_,
            mref<Vector<AccelerationStageInput>> substitutionInputs_,
            mref<Vector<AccelerationStageOutput>> substitutionOutputs_
        );

        ~AccelerationStageDerivat() override;

    private:
        Vector<AccelerationStageInput> _substitutionInputs;
        Vector<AccelerationStageOutput> _substitutionOutputs;

    public:
        [[nodiscard]] cref<Vector<AccelerationStageInput>> substitutionInputs() const noexcept;

        [[nodiscard]] cref<Vector<AccelerationStageOutput>> substitutionOutputs() const noexcept;

    private:
        nmpt<AccelerationStage> _owner;
        Vector<smr<AccelerationStage>> _supersedes;

    public:
        [[nodiscard]] nmpt<AccelerationStage> owner() const noexcept;

        [[nodiscard]] Vector<smr<AccelerationStage>> supersedes() const noexcept;

    public:
        void setStageFlagBits(mref<AccelerationStageFlagBits> flags_);

        void setStageModule(mref<smr<AccelerationStageModule>> module_);
    };
}
