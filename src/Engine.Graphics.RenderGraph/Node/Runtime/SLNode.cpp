#include "SLNode.hpp"

#include <Engine.Common/Make.hpp>

#include "../../Visitor/Visitor.hpp"

using namespace hg::engine::render::graph;
using namespace hg;

SLNode::SLNode() noexcept :
    RuntimeNode(make_smr<Auxiliary>()) {}

void SLNode::accept(ref<Visitor> visitor_) const {
    visitor_(*this);
}

void SLNode::traverse(ref<Visitor> visitor_) const {
    _next->accept(visitor_);
}

void SLNode::rtraverse(ref<Visitor> visitor_) const {
    _prev->accept(visitor_);
}

void SLNode::setNext(mref<smr<Node>> next_) {
    _next = std::move(next_);
}

smr<Node> SLNode::getNext() const noexcept {
    return _next;
}

void SLNode::setPrev(mref<nmpt<const Node>> prev_) {
    _prev = std::move(prev_);
}

nmpt<const Node> SLNode::getPrev() const noexcept {
    return _prev;
}
