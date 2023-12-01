#pragma once

#include "EffectCompileRequest.hpp"
#include "EffectCompileResult.hpp"
#include "Module/ModuleBuilder.hpp"
#include "Module/ModuleCompiler.hpp"
#include "Pass/PassBuilder.hpp"
#include "Pass/PassCompiler.hpp"
#include "Stage/StageComposer.hpp"

namespace hg::engine::accel {
    class EffectCompiler {
    public:
        using this_type = EffectCompiler;

    public:
        EffectCompiler() noexcept;

        EffectCompiler(
            mref<uptr<PassBuilder>> passBuilder_,
            mref<uptr<PassCompiler>> passCompiler_,
            mref<uptr<StageComposer>> stageComposer_,
            mref<uptr<ModuleBuilder>> moduleBuilder_,
            mref<uptr<ModuleCompiler>> moduleCompiler_
        ) noexcept;

        ~EffectCompiler() noexcept;

    private:
        uptr<PassBuilder> _passBuilder;
        uptr<PassCompiler> _passCompiler;

        uptr<StageComposer> _stageComposer;

        uptr<ModuleBuilder> _moduleBuilder;
        uptr<ModuleCompiler> _moduleCompiler;

    public:
        [[nodiscard]] EffectCompileResult compile(
            mref<EffectCompileRequest> request_
        ) const;
    };
}
