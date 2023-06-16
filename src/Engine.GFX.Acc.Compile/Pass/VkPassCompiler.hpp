#pragma once
#include "PassCompiler.hpp"

namespace hg::engine::gfx::acc {
    class VkPassCompiler final :
        public PassCompiler {
    public:
        using this_type = VkPassCompiler;

    public:
        VkPassCompiler();

        ~VkPassCompiler() override;

    public:
        smr<const AccelerationPass> compile(
            mref<smr<AccelerationPass>> source_,
            mref<Vector<uptr<CompiledModule>>> modules_
        ) const override;
    };
}
