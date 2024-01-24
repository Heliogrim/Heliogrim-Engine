#include "Renderer.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.GFX/Cache/LocalResourceCache.hpp>
#include <Engine.GFX/Cache/GlobalCacheCtrl.hpp>
#include <Engine.GFX/Memory/LocalPooledAllocator.hpp>
#include <Engine.GFX/Memory/GlobalPooledAllocator.hpp>
#include <Engine.GFX.RenderGraph.Compile/RenderGraphCompiler.hpp>
#include <Engine.GFX.RenderGraph/CompileGraph.hpp>
#include <Engine.GFX.RenderGraph/RuntimeGraph.hpp>
#include <Engine.GFX.RenderGraph/Pass/IterationPass.hpp>
#include <Engine.GFX.RenderGraph/Pass/ResolvePass.hpp>

#include "RenderPass.hpp"

using namespace hg::engine::render;
using namespace hg;

Renderer::Renderer(
    mref<Guid> guid_,
    mref<string> name_,
    mref<uptr<graph::CompileGraph>> compileGraph_,
    mref<smr<graph::InjectGraphRegistry>> injectionRegistry_,
    mref<nmpt<gfx::cache::GlobalCacheCtrl>> globalCache_,
    mref<nmpt<gfx::memory::GlobalPooledAllocator>> globalGfxAllocator_
) noexcept :
    _guid(std::move(guid_)),
    _name(std::move(name_)),
    _compileGraph(make_smr<>(std::move(compileGraph_))),
    _injectReg(std::move(injectionRegistry_)),
    _globalCache(std::move(globalCache_)),
    _globalGfxAlloc(std::move(globalGfxAllocator_)) {}

uptr<graph::RuntimeGraph> Renderer::makeDefaultRuntimeGraph() const {

    const auto expected = _compileGraph->expectedProvision();

    Vector<smr<const graph::Symbol>> defaultTargetSymbols {};
    defaultTargetSymbols.reserve(expected.size());

    for (const auto& provision : expected) {
        defaultTargetSymbols.push_back(provision.symbol);
    }

    constexpr graph::RenderGraphCompiler rgc {};
    return rgc(
        graph::CompileRequest {
            _compileGraph, std::move(defaultTargetSymbols)
        }
    );
}

uptr<RenderPass> Renderer::allocate() const {

    auto pass = make_uptr<RenderPass>(
        this,
        makeDefaultRuntimeGraph(),
        make_smr<gfx::cache::LocalCacheCtrl>(
            make_uptr<gfx::cache::LocalResourceCache>(_globalCache)
        ),
        make_smr<gfx::memory::LocalPooledAllocator>(_globalGfxAlloc)
    );

    // Note: We cannot to an implicit update ~> allocation due to missing external bound resources
    return pass;
}

std::pair<uptr<RenderPass>, bool> Renderer::reallocate(mref<uptr<RenderPass>> pass_) const {
    // Error: Will throw if used with initialized render pass due to missing external bound resources
    auto* const pass = pass_.get();
    return std::make_pair(std::move(pass_), pass->realloc());
}

void Renderer::free(mref<uptr<RenderPass>> pass_) const {
    pass_->free();
    pass_.reset();
}

uptr<RenderPass> Renderer::updateIncremental(mref<uptr<RenderPass>> pass_) const {

    auto& symbolContext = pass_->_state.rootSymbolContext();

    /* Update Runtime Graph */
    graph::IterationPass iterationPass {
        symbolContext
    };
    auto iteration = iterationPass.visitor();
    pass_->_graph->update(iteration);

    /* Resolve Runtime Graph */
    graph::ResolvePass resolvePass {
        symbolContext
    };
    auto resolve = resolvePass.visitor();
    pass_->_graph->update(resolve);

    // TODO: Check when to reallocate structured state objects

    /* Reallocate the pass to propagate changes */
    auto result = reallocate(std::move(pass_));
    return std::move(result.first);
}

uptr<RenderPass> Renderer::update(mref<uptr<RenderPass>> pass_) const {
    return updateIncremental(std::move(pass_));
}
