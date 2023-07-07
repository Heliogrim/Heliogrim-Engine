#include "AnchorNode.hpp"

#include <Engine.Common/Make.hpp>

#include "../Component/AnchorComponent.hpp"
#include "../Visitor/Visitor.hpp"

using namespace hg::engine::gfx::render::graph;
using namespace hg;

AnchorNode::AnchorNode() noexcept :
    Node(
        make_smr<Auxiliary>()
    ) {
    _auxiliary->add(make_uptr<AnchorComponent>());
}

void AnchorNode::accept(ref<Visitor> visitor_) const {
    visitor_(*this);
}

void AnchorNode::traverse(ref<Visitor> visitor_) const {
    if (empty()) {
        return;
    }

    _next->accept(visitor_);
}

bool AnchorNode::empty() const noexcept {
    return _next.empty();
}

void AnchorNode::setNext(mref<smr<Node>> next_) {
    _next = _STD move(next_);
}

smr<Node> AnchorNode::getNext() const noexcept {
    return _next;
}
