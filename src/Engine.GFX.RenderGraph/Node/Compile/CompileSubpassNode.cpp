#include "CompileSubpassNode.hpp"

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Make.hpp>

#include "../../Visitor/Visitor.hpp"

using namespace hg::engine::gfx::render::graph;
using namespace hg;

CompileSubpassNode::CompileSubpassNode() noexcept :
    CompileNode(
        make_smr<Auxiliary>()
    ) {}

void CompileSubpassNode::accept(ref<Visitor> visitor_) const {
    visitor_(*this);
}

void CompileSubpassNode::traverse(ref<Visitor> visitor_) const {
    _next->accept(visitor_);
}

void CompileSubpassNode::rtraverse(ref<Visitor> visitor_) const {
    _prev->accept(visitor_);
}

void CompileSubpassNode::setNext(mref<smr<Node>> next_) {
    _next = _STD move(next_);
}

smr<Node> CompileSubpassNode::getNext() const noexcept {
    return _next;
}

void CompileSubpassNode::setPrev(mref<nmpt<const Node>> prev_) {
    _prev = _STD move(prev_);
}

nmpt<const Node> CompileSubpassNode::getPrev() const noexcept {
    return _prev;
}
