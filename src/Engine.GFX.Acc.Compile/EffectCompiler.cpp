#include "EffectCompiler.hpp"

#include <Engine.GFX.Acc/AccelerationEffect.hpp>

#include "Module/ModuleBuilder.hpp"
#include "Module/ModuleCompiler.hpp"
#include "Stage/StageComposer.hpp"
#include "Token/ScopedTokenStorage.hpp"

using namespace hg::engine::gfx::acc;
using namespace hg;

EffectCompiler::EffectCompiler() noexcept = default;

EffectCompiler::EffectCompiler(
    mref<uptr<PassBuilder>> passBuilder_,
    mref<uptr<PassCompiler>> passCompiler_,
    mref<uptr<StageComposer>> stageComposer_,
    mref<uptr<ModuleBuilder>> moduleBuilder_,
    mref<uptr<ModuleCompiler>> moduleCompiler_,
    mref<Tokenizer> tokenizer_
) noexcept :
    _passBuilder(_STD move(passBuilder_)),
    _passCompiler(_STD move(passCompiler_)),
    _stageComposer(_STD move(stageComposer_)),
    _moduleBuilder(_STD move(moduleBuilder_)),
    _moduleCompiler(_STD move(moduleCompiler_)),
    _tokenizer(_STD move(tokenizer_)) {

    /**/
    _passCompiler->setTokenizer(&_tokenizer);
    _stageComposer->setTokenizer(&_tokenizer);
    _moduleBuilder->setTokenizer(&_tokenizer);
}

EffectCompiler::~EffectCompiler() noexcept = default;

smr<const AccelerationPass> EffectCompiler::compile(
    mref<EffectCompileRequest> request_
) const {

    /* Discover all tokens and resource dependencies */

    if (not request_.targetSymbols.empty()) {
        for (const auto& requested : request_.targetSymbols) {
            assert(_STD ranges::contains(request_.effect->getExportSymbols(), requested));
        }
    }

    /* Determine target pass type and spec based on effect */

    smr<AccelerationPass> pass = _passBuilder->build(
        _STD move(request_.effect),
        request_.spec,
        request_.profile
    );

    /* Compose a modified ordered list of stage */

    // TODO: Stage Composer should emit a set of Binding Layouts (vkDescriptorLayout)
    //  Binding layouts should be based on group and rate information, which should be present
    //  We might have to relate to exposed symbol properties, due to lack of information.

    // TODO: If there is a present set of targeted export symbols, we have to back-propagate those requirements. \
    //  Therefore, we are required to strip down all present stages of the provided effect, to just support the expected
    //  exports. If we have leftovers, we should trim them as well to prevent additional resource allocations or speculatively
    //  expose them. If no subset is provided, fast-forward the instructions and resources.

    // TODO: Check whether we want to generate the derivates before creating the target acceleration pass, cause we might \
    //  end up in a situation where we could strip down a complex shader-staging to a single compute instance.

    // Warning: We need to parse the shader code before the composing, cause composing is used to restructure and reduce the input.
    // Warning: Do we need to double parse the shader code? Using visual graph programming would give a an ir graph representation,
    // Warning:     which should be already enough; But textual shader input might not be transformable beforehand...

    // Accel Graph          -> IR \/
    // Accel Code -> Parser -> IR -> Compose Enrichment ( Profile Definitions, Specializations ) -> Composing ( Restructure, Reducing )
    //
    // Info: The created IR based on the graph or parse code might be incomplete, due to the profiles nature of providing definitions
    //          based on external dependencies.

    // After composing the actual stage derivates, we have to re-parse the generated code or consume the resulting IR and transform
    //  it into a IL which can be used by the spirv compiler. ( GLSL Vulkan Dialect )

    auto stages = _stageComposer->compose(
        request_.spec,
        request_.profile,
        request_.targetSymbols,
        pass
    );

    /* Build module sources out of the scoped stages */

    Vector<uptr<ModuleSource>> sources {};

    for (const auto& stageDerivat : stages) {

        constexpr uptr<ModuleSource> dummy {};
        const auto& previous = sources.empty() ? dummy : sources.back();

        // TODO: Module builder should consume layouts to resolved tokenized bindings into platform specific bindings
        // TODO: Module builder should emit a binding mapping per module source ~> compiled module to be able to query defined
        // TODO:    bindings by tokens downstream
        auto source = _moduleBuilder->build(pass, request_.spec, stageDerivat, previous);
        sources.push_back(_STD move(source));
    }

    /* Compile module sources to finite acceleration modules */

    Vector<uptr<CompiledModule>> modules {};

    for (auto&& source : sources) {

        auto cmodule = _moduleCompiler->compile(pass, request_.spec, _STD move(source));
        modules.push_back(_STD move(cmodule));
    }

    /* Finalize acceleration stage derivates */

    return _passCompiler->compile(request_.spec, _STD move(pass), _STD move(stages), _STD move(modules));
}
