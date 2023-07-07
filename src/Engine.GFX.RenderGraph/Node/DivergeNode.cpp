#include "DivergeNode.hpp"

#include "../Visitor/Visitor.hpp"

using namespace hg::engine::gfx::render::graph;
using namespace hg;

void DivergeNode::accept(ref<Visitor> visitor_) const {
    visitor_(*this);
}

void DivergeNode::traverse(ref<Visitor> visitor_) const {
    for (auto& child : _next) {
        child->accept(visitor_);
    }
}

void DivergeNode::addNext(mref<smr<Node>> next_) {
    _next.push_back(_STD move(next_));
}

void DivergeNode::setNext(mref<Vector<smr<Node>>> next_) {
    _next = _STD move(next_);
}

cref<Vector<smr<Node>>> DivergeNode::getNext() const noexcept {
    return _next;
}
