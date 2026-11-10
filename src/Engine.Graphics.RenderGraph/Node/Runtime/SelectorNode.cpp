#include "SelectorNode.hpp"

#include <Engine.Common/Make.hpp>

#include "../../Visitor/Visitor.hpp"

using namespace hg::engine::render::graph;
using namespace hg;

constexpr SelectorNode::mask_type boolToMask(bool active_) noexcept {
    SelectorNode::mask_type mask {};
    if (active_) {
        mask.set();
    }
    return mask;
}

/**/

SelectorNode::SelectorNode() noexcept :
    RuntimeNode(make_smr<Auxiliary>()) {}

void SelectorNode::accept(ref<Visitor> visitor_) const {
    visitor_(*this);
}

void SelectorNode::traverse(ref<Visitor> visitor_) const {

    constexpr mask_type off {};
    mask_type vmask {};

    for (const auto& child : _next) {
        if ((vmask & child.mask) != off) {
            child.node->accept(visitor_);
        }
    }
}

void SelectorNode::rtraverse(ref<Visitor> visitor_) const {
    _prev->accept(visitor_);
}

void SelectorNode::addNext(mask_type mask, mref<smr<Node>> next_) {
    _next.push_back(Next { mask, std::move(next_) });
}

void SelectorNode::addNext(bool active_, mref<smr<Node>> next_) {
    _next.push_back(Next { boolToMask(active_), std::move(next_) });
}

void SelectorNode::removeNext(cref<smr<Node>> next_) {
    _next.erase(
        std::remove_if(
            _next.begin(),
            _next.end(),
            [next_](const auto& child_) {
                return child_.node == next_;
            }
        ),
        _next.end()
    );
}

cref<Vector<SelectorNode::Next>> SelectorNode::getNext() const noexcept {
    return _next;
}

void SelectorNode::setPrev(mref<nmpt<const Node>> prev_) {
    _prev = std::move(prev_);
}

nmpt<const Node> SelectorNode::getPrev() const noexcept {
    return _prev;
}

IterationResult SelectorNode::iterate(cref<IterationPassContext> ctx_) noexcept {
    return IterationResultBits::eNone;
}

IterationResult SelectorNode::resolve(cref<ResolvePassContext> ctx_) noexcept {
    return IterationResultBits::eNone;
}

IterationResult SelectorNode::execute(cref<ExecutionPassContext> ctx_) noexcept {
    return IterationResultBits::eNone;
}
