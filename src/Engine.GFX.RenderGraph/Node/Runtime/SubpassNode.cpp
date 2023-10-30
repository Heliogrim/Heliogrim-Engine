#include "SubpassNode.hpp"

#include <Engine.Common/Make.hpp>

#include "../../Visitor/Visitor.hpp"

using namespace hg::engine::gfx::render::graph;
using namespace hg;

SubpassNode::SubpassNode() noexcept :
    RuntimeNode(make_smr<Auxiliary>()) {}

void SubpassNode::accept(ref<Visitor> visitor_) const {
    visitor_(*this);
}

void SubpassNode::traverse(ref<Visitor> visitor_) const {
    _next->accept(visitor_);
}

void SubpassNode::rtraverse(ref<Visitor> visitor_) const {
    _prev->accept(visitor_);
}

void SubpassNode::setNext(mref<smr<Node>> next_) {
    _next = _STD move(next_);
}

smr<Node> SubpassNode::getNext() const noexcept {
    return _next;
}

void SubpassNode::setPrev(mref<nmpt<const Node>> prev_) {
    _prev = _STD move(prev_);
}

nmpt<const Node> SubpassNode::getPrev() const noexcept {
    return _prev;
}
