#include "ResolverPass.hpp"

using namespace hg::engine::gfx::render::graph;
using namespace hg;

ResolverPass::ResolverPass(mref<ResolverPassFlags> flags_) noexcept :
    _flags(_STD move(flags_)) {}

ResolverPassFlags ResolverPass::getResolverFlags() const noexcept {
    return _flags;
}
