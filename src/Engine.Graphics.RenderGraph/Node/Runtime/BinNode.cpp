#include "BinNode.hpp"

using namespace hg::engine::render::graph;
using namespace hg;

IterationResult BinNode::iterate(cref<IterationPassContext> ctx_) noexcept {
    return ConvergeNode::iterate(ctx_);
}

IterationResult BinNode::resolve(cref<ResolvePassContext> ctx_) noexcept {
    return ConvergeNode::resolve(ctx_);
}

IterationResult BinNode::execute(cref<ExecutionPassContext> ctx_) noexcept {
    return ConvergeNode::execute(ctx_);
}
