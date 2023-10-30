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
        smr<const AccelerationPipeline> compile(
            cref<class EffectSpecification> specifications_,
            mref<smr<AccelerationPipeline>> source_,
            mref<Vector<smr<StageDerivat>>> stages_,
            mref<Vector<uptr<class CompiledModule>>> modules_
        ) const override;
    };
}
