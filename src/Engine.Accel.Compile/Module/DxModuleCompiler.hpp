#pragma once
#include "ModuleCompiler.hpp"

namespace hg::engine::accel {
    class DxModuleCompiler final :
        public ModuleCompiler {
    public:
        using this_type = DxModuleCompiler;

    public:
        DxModuleCompiler() = default;

        ~DxModuleCompiler() noexcept override;

    public:
        [[nodiscard]] uptr<CompiledModule> compile(
            cref<smr<AccelerationPipeline>> targetPass_,
            cref<smr<const class EffectSpecification>> specifications_,
            mref<uptr<ModuleSource>> source_
        ) const override;
    };
}
