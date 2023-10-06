#pragma once

#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.GFX.Acc/AccelerationPass.hpp>

#include "EffectCompileRequest.hpp"
#include "Module/ModuleBuilder.hpp"
#include "Module/ModuleCompiler.hpp"
#include "Pass/PassBuilder.hpp"
#include "Pass/PassCompiler.hpp"
#include "Profile/EffectProfile.hpp"
#include "Spec/SpecificationStorage.hpp"
#include "Stage/StageComposer.hpp"
#include "Token/Tokenizer.hpp"

namespace hg::engine::gfx::acc {
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
            mref<uptr<ModuleCompiler>> moduleCompiler_,
            mref<Tokenizer> tokenizer_
        ) noexcept;

        ~EffectCompiler() noexcept;

    private:
        uptr<PassBuilder> _passBuilder;
        uptr<PassCompiler> _passCompiler;

        uptr<StageComposer> _stageComposer;

        uptr<ModuleBuilder> _moduleBuilder;
        uptr<ModuleCompiler> _moduleCompiler;

        Tokenizer _tokenizer;

    public:
        [[nodiscard]] smr<const AccelerationPass> compile(
            mref<EffectCompileRequest> request_
        ) const;
    };
}
