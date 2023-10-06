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
    _auxiliary->add(uptr<AnchorComponent>(new AnchorComponent()));
}

AnchorNode::~AnchorNode() noexcept = default;

void AnchorNode::accept(ref<Visitor> visitor_) const {
    visitor_(*this);
}

void AnchorNode::traverse(ref<Visitor> visitor_) const {
    if (!hasNext()) {
        return;
    }

    _next->accept(visitor_);
}

void AnchorNode::rtraverse(ref<Visitor> visitor_) const {
    if (!hasPrev()) {
        return;
    }

    _prev->accept(visitor_);
}

bool AnchorNode::hasNext() const noexcept {
    return not _next.empty();
}

void AnchorNode::setNext(mref<smr<Node>> next_) {
    _next = _STD move(next_);
}

smr<Node> AnchorNode::getNext() const noexcept {
    return _next;
}

bool AnchorNode::hasPrev() const noexcept {
    return not _prev.empty();
}

void AnchorNode::setPrev(mref<smr<Node>> prev_) {
    _prev = _STD move(prev_);
}

smr<Node> AnchorNode::getPrev() const noexcept {
    return _prev;
}

nmpt<const AnchorComponent> AnchorNode::getAnchorComponent() const noexcept {
    return const_cast<ptr<this_type>>(this)->getAnchorComponent();
}

nmpt<AnchorComponent> AnchorNode::getAnchorComponent() {
    const auto it = _STD ranges::find_if(
        _auxiliary->components(),
        [](const ptr<Component> component_) {
            return component_->getMetaClass()->exact<AnchorComponent>();
        }
    );
    return it != _auxiliary->components().end() ?
               static_cast<ptr<AnchorComponent>>(*it) :
               nmpt<AnchorComponent> {};
}
