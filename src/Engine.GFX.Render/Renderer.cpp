#include "Renderer.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.GFX/Cache/LocalResourceCache.hpp>
#include <Engine.GFX/Cache/GlobalCacheCtrl.hpp>
#include <Engine.GFX/Memory/LocalPooledAllocator.hpp>
#include <Engine.GFX/Memory/GlobalPooledAllocator.hpp>
#include <Engine.GFX.RenderPipeline/RenderPipeline.hpp>
#include <Engine.GFX.RenderPipeline/State/State.hpp>
#include <Engine.GFX.RenderGraph.Compile/RenderGraphCompiler.hpp>
#include <Engine.GFX.RenderGraph/CompileGraph.hpp>
#include <Engine.GFX.RenderGraph/RuntimeGraph.hpp>
#include <Engine.GFX.RenderGraph/Pass/IterationPass.hpp>
#include <Engine.GFX.RenderGraph/Pass/ResolvePass.hpp>
#include <Engine.GFX.RenderPipeline/Build/BaseBuilder.hpp>
#include <Engine.GFX.RenderPipeline/Build/StreamBuilder.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>

#include "RenderPass.hpp"

using namespace hg::engine::gfx::render;
using namespace hg;

Renderer::Renderer(
    mref<Guid> guid_,
    mref<string> name_,
    mref<uptr<graph::CompileGraph>> compileGraph_,
    mref<smr<graph::InjectGraphRegistry>> injectionRegistry_,
    mref<nmpt<cache::GlobalCacheCtrl>> globalCache_,
    mref<nmpt<memory::GlobalPooledAllocator>> globalGfxAllocator_
) noexcept :
    _guid(_STD move(guid_)),
    _name(_STD move(name_)),
    _compileGraph(make_smr<>(_STD move(compileGraph_))),
    _injectReg(_STD move(injectionRegistry_)),
    _globalCache(_STD move(globalCache_)),
    _globalGfxAlloc(_STD move(globalGfxAllocator_)) {}

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

uptr<RenderPass> Renderer::allocate() const {

    auto pass = make_uptr<RenderPass>(
        this,
        makeDefaultRuntimeGraph(),
        make_smr<cache::LocalCacheCtrl>(
            make_uptr<cache::LocalResourceCache>(_globalCache)
        ),
        make_smr<memory::LocalPooledAllocator>(_globalGfxAlloc)
    );

    // Note: We cannot to an implicit update ~> allocation due to missing external bound resources
    return pass;
}

_STD pair<uptr<RenderPass>, bool> Renderer::reallocate(mref<uptr<RenderPass>> pass_) const {
    // Error: Will throw if used with initialized render pass due to missing external bound resources
    auto* const pass = pass_.get();
    return _STD make_pair(_STD move(pass_), pass->realloc());
}

void Renderer::free(mref<uptr<RenderPass>> pass_) const {
    pass_->free();
    pass_.reset();
}

uptr<RenderPass> Renderer::updateIncremental(mref<uptr<RenderPass>> pass_) const {

    /* Update Runtime Graph */
    graph::IterationPass iterationPass {};
    auto iteration = iterationPass.visitor();
    pass_->_graph->update(iteration);

    /* Resolve Runtime Graph */
    graph::ResolvePass resolvePass {};
    auto resolve = resolvePass.visitor();
    pass_->_graph->update(resolve);

    // TODO: Check when to reallocate structured state objects

    /* Reallocate the pass to propagate changes */
    auto result = reallocate(_STD move(pass_));
    return _STD move(result.first);
}

uptr<RenderPass> Renderer::update(mref<uptr<RenderPass>> pass_) const {
    return updateIncremental(_STD move(pass_));
}
