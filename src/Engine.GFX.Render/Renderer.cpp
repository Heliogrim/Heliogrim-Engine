#include "Renderer.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.GFX/Cache/LocalResourceCache.hpp>
#include <Engine.GFX/Cache/GlobalCacheCtrl.hpp>
#include <Engine.GFX/Memory/LocalPooledAllocator.hpp>
#include <Engine.GFX/Memory/GlobalPooledAllocator.hpp>
#include <Engine.GFX.RenderPipeline/RenderPipeline.hpp>
#include <Engine.GFX.RenderPipeline/State/State.hpp>
#include <Engine.GFX.RenderGraph/Resolver/Resolver.hpp>
#include <Engine.GFX.RenderGraph.Compile/RenderGraphCompiler.hpp>
#include <Engine.GFX.RenderGraph/CompileGraph.hpp>
#include <Engine.GFX.RenderGraph/RuntimeGraph.hpp>
#include <Engine.GFX.RenderPipeline/Build/BaseBuilder.hpp>
#include <Engine.GFX.RenderPipeline/Build/StreamBuilder.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>

#include "RenderPass.hpp"

using namespace hg::engine::gfx::render;
using namespace hg;

Renderer::Renderer(
    mref<Guid> guid_,
    mref<string> name_,
    u32 runtimeVersion_,
    mref<uptr<graph::CompileGraph>> compileGraph_,
    mref<smr<graph::Resolver>> baseResolver_,
    mref<smr<graph::InjectGraphRegistry>> injectionRegistry_,
    mref<smr<cache::GlobalCacheCtrl>> globalCache_,
    mref<smr<memory::GlobalPooledAllocator>> globalGfxAllocator_
) noexcept :
    _guid(_STD move(guid_)),
    _name(_STD move(name_)),
    _rtVer(runtimeVersion_),
    _compileGraph(make_smr<>(_STD move(compileGraph_))),
    _baseRes(_STD move(baseResolver_)),
    _injectReg(_STD move(injectionRegistry_)),
    _globalCache(_STD move(globalCache_)),
    _globalGfxAlloc(_STD move(globalGfxAllocator_)) {}

smr<graph::Resolver> Renderer::makeAdvancedResolver() const {
    return make_smr<graph::Resolver>();
}

uptr<graph::RuntimeGraph> Renderer::makeDefaultRuntimeGraph() const {

    const auto expected = _compileGraph->expectedProvision();

    Vector<smr<const acc::Symbol>> defaultTargetSymbols {};
    defaultTargetSymbols.reserve(expected.size());

    for (const auto& provision : expected) {
        defaultTargetSymbols.push_back(provision.symbol);
    }

    constexpr graph::RenderGraphCompiler rgc {};
    return rgc(
        graph::CompileRequest {
            _compileGraph, _STD move(defaultTargetSymbols)
        }
    );
}

uptr<RenderPipeline> Renderer::makeRenderPipeline() const {

    constexpr auto builder = pipeline::BaseBuilder();
    auto runtimeGraph = makeDefaultRuntimeGraph();

    return builder(_STD move(runtimeGraph));
}

uptr<pipeline::State> Renderer::makePipelineState(mref<nmpt<const RenderPipeline>> pipeline_) const {

    //auto state = make_uptr<pipeline::State>(
    //    make_uptr<cache::LocalResourceCache>(_globalCache.get()),
    //    memory::LocalPooledAllocator { _globalGfxAlloc.get() }
    //);
    auto state = make_uptr<pipeline::State>();

    /**/

    return updatePipelineState(_STD move(pipeline_), _STD move(state));
}

uptr<pipeline::State> Renderer::updatePipelineState(
    mref<nmpt<const RenderPipeline>> pipeline_,
    mref<uptr<pipeline::State>> state_
) const {
    pipeline_->regSubState(state_.get());
    return state_;
}

uptr<RenderPass> Renderer::allocate() const {

    auto pipe = makeRenderPipeline();
    auto state = makePipelineState(pipe.get());

    auto pass = make_uptr<RenderPass>(
        _rtVer /* force invalidation */ - 1ui32,
        _STD move(pipe),
        _STD move(state)
    );

    /**/

    if (not pass->alloc()) {
        /* Failover */
        free(_STD move(pass));
        return nullptr;
    }

    /**/

    return updateOnDelta(_STD move(pass));
}

_STD pair<uptr<RenderPass>, bool> Renderer::reallocate(mref<uptr<RenderPass>> pass_) const {
    auto* const pass = pass_.get();
    return _STD make_pair(_STD move(pass_), pass->realloc());
}

void Renderer::free(mref<uptr<RenderPass>> pass_) const {
    pass_->free();
    pass_.reset();
}

uptr<RenderPass> Renderer::updateIncremental(
    mref<uptr<RenderPass>> pass_,
    mref<uptr<graph::RuntimeGraph>> nextGraph_
) const {

    /* Update Pipeline */
    constexpr auto builder = pipeline::StreamBuilder();
    pass_->_pipeline = builder(_STD move(pass_->_pipeline), _STD move(nextGraph_));

    /* Update Pipeline State sub-sequential */
    pass_->_state = updatePipelineState(pass_->_pipeline.get(), _STD move(pass_->_state));

    /* Reallocate the pass to propagate changes */
    auto result = reallocate(_STD move(pass_));
    return _STD move(result.first);
}

uptr<RenderPass> Renderer::updateOnDelta(mref<uptr<RenderPass>> pass_) const {

    /* Warning: We currently use CAS for integrity check, but we won't advice concurrent usage of updateOnDelta with same pass object. */
    auto expect = _rtVer.load(_STD memory_order::relaxed);
    if (pass_->_rtVer.compare_exchange_strong(expect, expect, _STD memory_order::acq_rel)) {
        return pass_;
    }

    /**/

    // TODO: clone source graph to working set
    // TODO: Check whether we want to use the original graph as immutable source and build a new graph
    // TODO:    using the advanced resolver to prevent plane copies
    // Warning: Duplicating the graph and the internal state is a lot of work...
    auto base = make_uptr<graph::RuntimeGraph>();
    //auto base = make_uptr<render::RuntimeGraph>(*_compileGraph);

    auto resolver = makeAdvancedResolver();
    auto resolverOptions = graph::ResolverOptions {};

    auto nextGraph = (*resolver)(_STD move(base), _STD move(resolverOptions));

    /**/

    return updateIncremental(_STD move(pass_), _STD move(nextGraph));
}

/**/

void test() {

    Renderer* renderer = nullptr;
    auto pass = renderer->allocate();
    //auto [next, result] = renderer->reallocate(_STD move(pass));

    bool interupted = false;
    while (not interupted) {
        pass = renderer->updateOnDelta(_STD move(pass));

        const auto result = (*pass)();
        interupted = result != RenderPassResult::eFailed;
    }

    renderer->free(_STD move(pass));
}
