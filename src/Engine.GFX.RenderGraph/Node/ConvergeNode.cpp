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

void ConvergeNode::rtraverse(ref<Visitor> visitor_) const {
    auto prev = _prev;
    for (const auto& parent : prev) {
        parent->accept(visitor_);
    }
}

void ConvergeNode::setNext(mref<smr<Node>> next_) {
    _next = _STD move(next_);
}

smr<Node> ConvergeNode::getNext() const noexcept {
    return _next;
}

void ConvergeNode::addPrev(mref<smr<Node>> prev_) {
    _prev.push_back(_STD move(prev_));
}

void ConvergeNode::setPrev(mref<Vector<smr<Node>>> prev_) {
    _prev = _STD move(prev_);
}

void ConvergeNode::removePrev(cref<smr<Node>> prev_) {
    _prev.erase(
        _STD remove_if(
            _prev.begin(),
            _prev.end(),
            [prev_](const auto node_) {
                return node_ == prev_;
            }
        ),
        _prev.end()
    );
}

cref<Vector<smr<Node>>> ConvergeNode::getPrev() const noexcept {
    return _prev;
}
