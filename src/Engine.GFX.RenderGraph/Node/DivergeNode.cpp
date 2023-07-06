#include "DivergeNode.hpp"

using namespace hg::engine::gfx::render::graph;
using namespace hg;

void DivergeNode::traverse(ref<Visitor> visitor_) {
    for (auto& child : _next) {
        child->accept(visitor_);
    }
}

void DivergeNode::addNext(mref<smr<Node>> next_) {
    _next.push_back(_STD move(next_));
}
