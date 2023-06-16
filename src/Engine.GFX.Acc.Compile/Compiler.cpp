#include "Compiler.hpp"

#include <Engine.GFX.Acc/AccelerationEffect.hpp>

#include "Module/ModuleBuilder.hpp"
#include "Module/ModuleCompiler.hpp"
#include "Stage/StageComposer.hpp"
#include "Token/ScopedTokenStorage.hpp"

using namespace hg::engine::gfx::acc;
using namespace hg;

static void discoverInputLayout(
    cref<Tokenizer> tokenizer_,
    cref<smr<AccelerationEffect>> effect_,
    _Inout_ ref<ScopedTokenStorage> sts_
);

static void discoverBindingLayout(
    cref<Tokenizer> tokenizer_,
    cref<smr<AccelerationEffect>> effect_,
    _Inout_ ref<ScopedTokenStorage> sts_
);

static void discoverOutputLayout(
    cref<Tokenizer> tokenizer_,
    cref<smr<AccelerationEffect>> effect_,
    _Inout_ ref<ScopedTokenStorage> sts_
);

static void discoverStages(
    cref<Tokenizer> tokenizer_,
    cref<smr<AccelerationEffect>> effect_,
    _Inout_ ref<ScopedTokenStorage> sts_
);

static void discoverStageInput(
    cref<Tokenizer> tokenizer_,
    cref<smr<AccelerationStage>> stage_,
    _Inout_ ref<ScopedTokenStorage> sts_
);

static void discoverStageOutput(
    cref<Tokenizer> tokenizer_,
    cref<smr<AccelerationStage>> stage_,
    _Inout_ ref<ScopedTokenStorage> sts_
);

Compiler::Compiler() noexcept = default;

Compiler::~Compiler() noexcept = default;

smr<const AccelerationPass> Compiler::compile(mref<smr<AccelerationEffect>> effect_) const {

    /* Discover all tokens and resource dependencies */

    // TODO: Discover resource dependencies (external bindings + module inter-dependencies)

    ScopedTokenStorage sts {};
    discoverInputLayout(_tokenizer, effect_, sts);
    discoverBindingLayout(_tokenizer, effect_, sts);
    discoverOutputLayout(_tokenizer, effect_, sts);

    discoverStages(_tokenizer, effect_, sts);

    /* Determine target pass type and spec based on effect */

    smr<AccelerationPass> pass = _passBuilder->build(_STD move(effect_));

    /* Compose a modified ordered list of stage */

    auto stages = _stageComposer->compose(pass, sts);

    /* Build module sources out of the scoped stages */

    Vector<ModuleSource> sources {};

    for (const auto& stageDerivat : stages) {

        // TODO: Generate subset of token accessible at current stage
        ScopedTokenStorage csts {};

        auto source = _moduleBuilder->build(pass, csts, stageDerivat);
        sources.push_back(_STD move(source));
    }

    /* Compile module sources to finite acceleration modules */

    Vector<uptr<CompiledModule>> modules {};

    for (auto&& source : sources) {

        auto cmodule = _moduleCompiler->compile(pass, _STD move(source));
        modules.push_back(_STD move(cmodule));
    }

    /* Finalize acceleration stage derivates */

    return _passCompiler->compile(_STD move(pass), _STD move(modules));
}

/**/
void discoverInputLayout(
    cref<Tokenizer> tokenizer_,
    cref<smr<AccelerationEffect>> effect_,
    ref<ScopedTokenStorage> sts_
) {
    const auto& inputLayout = effect_->getInputLayout();
    for (const auto& description : inputLayout.getDescriptions()) {
        auto token = tokenizer_.generate(description);
        sts_.addToken(_STD move(token));
    }
}

void discoverBindingLayout(
    cref<Tokenizer> tokenizer_,
    cref<smr<AccelerationEffect>> effect_,
    ref<ScopedTokenStorage> sts_
) {
    const auto& bindingLayout = effect_->getBindingLayout();
    for (const auto& description : bindingLayout.getDescriptions()) {
        auto token = tokenizer_.generate(description);
        sts_.addToken(_STD move(token));
    }
}

void discoverOutputLayout(
    cref<Tokenizer> tokenizer_,
    cref<smr<AccelerationEffect>> effect_,
    ref<ScopedTokenStorage> sts_
) {
    const auto& outputLayout = effect_->getOutputLayout();
    for (const auto& description : outputLayout.getDescriptions()) {
        auto token = tokenizer_.generate(description);
        sts_.addToken(_STD move(token));
    }
}

void discoverStages(cref<Tokenizer> tokenizer_, cref<smr<AccelerationEffect>> effect_, ref<ScopedTokenStorage> sts_) {
    for (const auto& stage : effect_->getStages()) {
        discoverStageInput(tokenizer_, stage, sts_);
        discoverStageOutput(tokenizer_, stage, sts_);
    }
}

void discoverStageInput(
    cref<Tokenizer> tokenizer_,
    cref<smr<AccelerationStage>> stage_,
    ref<ScopedTokenStorage> sts_
) {
    for (const auto& stageInput : stage_->getStageInputs()) {
        auto token = tokenizer_.generate(stageInput);
        sts_.addToken(_STD move(token));
    }
}

void discoverStageOutput(
    cref<Tokenizer> tokenizer_,
    cref<smr<AccelerationStage>> stage_,
    ref<ScopedTokenStorage> sts_
) {
    for (const auto& stageOutput : stage_->getStageOutputs()) {
        auto token = tokenizer_.generate(stageOutput);
        sts_.addToken(_STD move(token));
    }
}
