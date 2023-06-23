#pragma once

#include <Engine.GFX.Acc/AccelerationPass.hpp>

#include "../Module/CompiledModule.hpp"

namespace hg::engine::gfx::acc {
    class __declspec(novtable) PassCompiler {
    public:
        using this_type = PassCompiler;

    public:
        PassCompiler() noexcept = default;

        virtual ~PassCompiler() noexcept = default;

    public:
        [[nodiscard]] _Success_(return != nullptr) virtual smr<const AccelerationPass> compile(
            mref<smr<AccelerationPass>> source_,
            mref<Vector<smr<AccelerationStageDerivat>>> stages_,
            mref<Vector<uptr<CompiledModule>>> modules_
        ) const = 0;
    };
}
