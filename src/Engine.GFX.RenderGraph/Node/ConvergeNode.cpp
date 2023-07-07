#include "ConvergeNode.hpp"

#include "../Visitor/Visitor.hpp"

using namespace hg::engine::gfx::render::graph;
using namespace hg;

void ConvergeNode::accept(ref<Visitor> visitor_) const {
    visitor_(*this);
}

void ConvergeNode::traverse(ref<Visitor> visitor_) const {
    _next->accept(visitor_);
}

bool ConvergeNode::empty() const noexcept {
    return _next.empty();
}

void ConvergeNode::setNext(mref<smr<Node>> next_) {
    _next = _STD move(next_);
}

smr<Node> ConvergeNode::getNext() const noexcept {
    return _next;
}
