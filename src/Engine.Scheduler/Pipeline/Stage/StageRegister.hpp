#pragma once

#include "Stage.hpp"

namespace hg::engine::scheduler {
    class PipelineStage;
}

namespace hg::engine::scheduler {
    class __declspec(novtable) StageRegister {
    public:
        using this_type = StageRegister;

    public:
        virtual ~StageRegister() = default;

    public:
        [[nodiscard]] virtual const non_owning_rptr<const Stage> registerStage(mref<uptr<PipelineStage>> stage_) = 0;

        [[nodiscard]] virtual const non_owning_rptr<const Stage> getStage(cref<string> identifier_) const noexcept = 0;

        virtual void removeStage(cref<string> identifier_) = 0;

        virtual void removeStage(mref<non_owning_rptr<Stage>> stage_) = 0;
    };
}
