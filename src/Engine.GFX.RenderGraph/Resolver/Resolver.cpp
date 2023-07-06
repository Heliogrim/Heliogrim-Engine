#include "Resolver.hpp"

#include "ResolverPass.hpp"
#include "../RenderGraph.hpp"

using namespace hg::engine::gfx::render::graph;
using namespace hg::engine::gfx::render;
using namespace hg;

uptr<RenderGraph> Resolver::operator()(
    mref<uptr<RenderGraph>> graph_,
    mref<ResolverOptions> options_
) const noexcept {

    uptr<RenderGraph> graph = _STD move(graph_);
    for (const auto& pass : _passes) {
        if (pass->getResolverFlags() & options_.flags) {
            graph = (*pass)(_STD move(graph));
        }
    }

    return _STD move(graph);
}
