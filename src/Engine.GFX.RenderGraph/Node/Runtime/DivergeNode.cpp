#include "DivergeNode.hpp"

#include <Engine.Common/Make.hpp>

#include "../../Visitor/Visitor.hpp"

using namespace hg::engine::render::graph;
using namespace hg;

DivergeNode::DivergeNode() noexcept :
    RuntimeNode(make_smr<Auxiliary>()) {}

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

void DivergeNode::setPrev(mref<nmpt<const Node>> prev_) {
    _prev = _STD move(prev_);
}

nmpt<const Node> DivergeNode::getPrev() const noexcept {
    return _prev;
}

IterationResult DivergeNode::iterate(cref<IterationPassContext> ctx_) noexcept {
    return IterationResultBits::eNone;
}

IterationResult DivergeNode::resolve(cref<ResolvePassContext> ctx_) noexcept {
    return IterationResultBits::eNone;
}

IterationResult DivergeNode::execute(cref<ExecutionPassContext> ctx_) noexcept {
    return IterationResultBits::eNone;
}
