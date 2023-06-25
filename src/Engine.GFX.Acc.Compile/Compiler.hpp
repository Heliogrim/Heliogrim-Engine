#pragma once

#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.GFX.Acc/AccelerationPass.hpp>

#include "Module/ModuleBuilder.hpp"
#include "Module/ModuleCompiler.hpp"
#include "Pass/PassBuilder.hpp"
#include "Pass/PassCompiler.hpp"
#include "Spec/SpecificationStorage.hpp"
#include "Stage/StageComposer.hpp"
#include "Token/Tokenizer.hpp"

namespace hg::engine::gfx::acc {
    class Compiler {
    public:
        using this_type = Compiler;

    public:
        Compiler() noexcept;

        Compiler(
            mref<uptr<PassBuilder>> passBuilder_,
            mref<uptr<PassCompiler>> passCompiler_,
            mref<uptr<StageComposer>> stageComposer_,
            mref<uptr<ModuleBuilder>> moduleBuilder_,
            mref<uptr<ModuleCompiler>> moduleCompiler_,
            mref<Tokenizer> tokenizer_
        ) noexcept;

        ~Compiler() noexcept;

    private:
        uptr<PassBuilder> _passBuilder;
        uptr<PassCompiler> _passCompiler;

        uptr<StageComposer> _stageComposer;

        uptr<ModuleBuilder> _moduleBuilder;
        uptr<ModuleCompiler> _moduleCompiler;

        Tokenizer _tokenizer;

    public:
        [[nodiscard]] smr<const AccelerationPass> compile(
            mref<smr<AccelerationEffect>> effect_,
            cref<SpecificationStorage> specifications_
        ) const;
    };
}
