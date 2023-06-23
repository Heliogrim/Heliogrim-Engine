#pragma once
#include "PassCompiler.hpp"

namespace hg::engine::gfx::acc {
    class DxPassCompiler final :
        public PassCompiler {
    public:
        using this_type = DxPassCompiler;

    public:
        DxPassCompiler();

        ~DxPassCompiler() override;

    public:
        smr<const AccelerationPass> compile(
            mref<smr<AccelerationPass>> source_,
            mref<Vector<smr<AccelerationStageDerivat>>> stages_,
            mref<Vector<uptr<CompiledModule>>> modules_
        ) const override;
    };
}
