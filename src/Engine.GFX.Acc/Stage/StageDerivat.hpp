#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

#include "Stage.hpp"

namespace hg::engine::gfx::acc {
    class StageDerivat :
        public Stage {
    public:
        using this_type = StageDerivat;

    public:
        StageDerivat() noexcept;

        StageDerivat(
            mref<nmpt<Stage>> owner_,
            mref<Vector<smr<Stage>>> supersedes_,
            mref<smr<StageModule>> stageModule_,
            mref<StageFlagBits> flagBits_,
            mref<Vector<StageInput>> stageInputs_,
            mref<Vector<StageOutput>> stageOutputs_,
            mref<Vector<StageInput>> substitutionInputs_,
            mref<Vector<StageOutput>> substitutionOutputs_
        );

        ~StageDerivat() override;

    private:
        Vector<StageInput> _substitutionInputs;
        Vector<StageOutput> _substitutionOutputs;

    public:
        [[nodiscard]] cref<Vector<StageInput>> substitutionInputs() const noexcept;

        [[nodiscard]] cref<Vector<StageOutput>> substitutionOutputs() const noexcept;

    private:
        nmpt<Stage> _owner;
        Vector<smr<Stage>> _supersedes;

    public:
        [[nodiscard]] nmpt<Stage> owner() const noexcept;

        [[nodiscard]] Vector<smr<Stage>> supersedes() const noexcept;

    private:
        smr<StageModule> _stageModule;

    public:
        void setStageFlagBits(mref<StageFlagBits> flags_);

        void setStageModule(mref<smr<StageModule>> module_);

        [[nodiscard]] smr<StageModule> getStageModule() const noexcept;
    };
}
