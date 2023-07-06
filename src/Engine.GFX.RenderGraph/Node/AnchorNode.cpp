#include "AnchorNode.hpp"

#include <Engine.Common/Make.hpp>

#include "../Component/AnchorComponent.hpp"

using namespace hg::engine::gfx::render::graph;
using namespace hg;

AnchorNode::AnchorNode() noexcept :
    Node(
        make_smr<Auxiliary>()
    ) {
    _auxiliary->add(make_uptr<AnchorComponent>());
}

void AnchorNode::traverse(ref<Visitor> visitor_) {
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
