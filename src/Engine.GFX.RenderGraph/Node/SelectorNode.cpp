#include "SelectorNode.hpp"

#include "../Visitor/Visitor.hpp"

using namespace hg::engine::gfx::render::graph;
using namespace hg;

constexpr SelectorNode::mask_type boolToMask(bool active_) noexcept {
    SelectorNode::mask_type mask {};
    if (active_) {
        mask.set();
    }
    return mask;
}

/**/

void SelectorNode::accept(ref<Visitor> visitor_) const {
    visitor_(*this);
}

void SelectorNode::traverse(ref<Visitor> visitor_) const {

    constexpr mask_type off {};
    mask_type vmask {};

    for (auto& child : _next) {
        if ((vmask & child.mask) != off) {
            child.node->accept(visitor_);
        }
    }
}

void SelectorNode::addNext(mask_type mask, mref<smr<Node>> next_) {
    _next.push_back(Next { mask, _STD move(next_) });
}

void SelectorNode::addNext(bool active_, mref<smr<Node>> next_) {
    _next.push_back(Next { boolToMask(active_), _STD move(next_) });
}

cref<Vector<SelectorNode::Next>> SelectorNode::getNext() const noexcept {
    return _next;
}
