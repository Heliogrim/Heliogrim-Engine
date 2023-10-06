#include "DivergeNode.hpp"

#include "../Visitor/Visitor.hpp"

using namespace hg::engine::gfx::render::graph;
using namespace hg;

void DivergeNode::accept(ref<Visitor> visitor_) const {
    visitor_(*this);
}

void DivergeNode::traverse(ref<Visitor> visitor_) const {
    auto next = _next;
    for (auto& child : next) {
        child->accept(visitor_);
    }
}

void DivergeNode::rtraverse(ref<Visitor> visitor_) const {
    _prev->accept(visitor_);
}

void DivergeNode::addNext(mref<smr<Node>> next_) {
    _next.push_back(_STD move(next_));
}

void DivergeNode::setNext(mref<Vector<smr<Node>>> next_) {
    _next = _STD move(next_);
}

void DivergeNode::removeNext(cref<smr<Node>> next_) {
    _next.erase(
        _STD remove_if(
            _next.begin(),
            _next.end(),
            [next_](const auto node_) {
                return node_ == next_;
            }
        ),
        _next.end()
    );
}

cref<Vector<smr<Node>>> DivergeNode::getNext() const noexcept {
    return _next;
}

void DivergeNode::setPrev(mref<smr<Node>> prev_) {
    _prev = _STD move(prev_);
}

smr<Node> DivergeNode::getPrev() const noexcept {
    return _prev;
}
