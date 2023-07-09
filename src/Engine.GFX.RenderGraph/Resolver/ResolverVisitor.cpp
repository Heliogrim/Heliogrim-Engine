#include "ResolverVisitor.hpp"

#include "../Node/Node.hpp"

using namespace hg::engine::gfx::render::graph;
using namespace hg;

ResolverVisitor::~ResolverVisitor() noexcept = default;

void ResolverVisitor::operator()(cref<Node> node_) {
    node_.traverse(*this);
}
