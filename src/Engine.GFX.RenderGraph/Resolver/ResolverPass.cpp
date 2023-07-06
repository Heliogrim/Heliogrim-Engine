#include "ResolverPass.hpp"

using namespace hg::engine::gfx::render::graph;
using namespace hg;

ResolverPassFlags ResolverPass::getResolverFlags() const noexcept {
    return _flags;
}
