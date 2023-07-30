#include "Node.hpp"

#include "../Visitor/Visitor.hpp"

using namespace hg::engine::gfx::render::graph;
using namespace hg;

Node::Node(mref<smr<Auxiliary>> auxiliary_) noexcept :
    _auxiliary(_STD move(auxiliary_)) {}

void Node::auxAdd(mref<uptr<Component>> comp_) {

    comp_->mount(this);
    for (const auto& component : _auxiliary->components()) {
        component->mounted(comp_.get());
    }

    _auxiliary->add(_STD move(comp_));
}

void Node::auxRem(mref<nmpt<Component>> comp_) {

    comp_->unmount(this);
    for (const auto& component : _auxiliary->components()) {
        component->unmounted(comp_);
    }

    // Warning: This will delete the element.
    _auxiliary->remove(comp_.get());
}

smr<Auxiliary> Node::auxiliary() const noexcept {
    return _auxiliary;
}
