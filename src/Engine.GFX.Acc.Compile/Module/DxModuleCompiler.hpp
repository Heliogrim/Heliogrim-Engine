#pragma once
#include "ModuleCompiler.hpp"

namespace hg::engine::gfx::acc {
    class DxModuleCompiler final :
        public ModuleCompiler {
    public:
        using this_type = DxModuleCompiler;

    public:
        DxModuleCompiler() = default;

        ~DxModuleCompiler() noexcept override;

    public:
        [[nodiscard]] CompiledModule compile(
            cref<smr<AccelerationPass>> targetPass_,
            mref<ModuleSource> source_
        ) const override;
    };
}
