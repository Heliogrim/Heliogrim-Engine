#include "ConvergeNode.hpp"

using namespace hg::engine::gfx::render::graph;
using namespace hg;

void ConvergeNode::traverse(ref<Visitor> visitor_) {
    _next->accept(visitor_);
}

bool ConvergeNode::empty() const noexcept {
    return _next.empty();
}

void ConvergeNode::setNext(mref<smr<Node>> next_) {
    _next = _STD move(next_);
}
