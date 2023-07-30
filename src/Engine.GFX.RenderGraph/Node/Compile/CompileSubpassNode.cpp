#include "CompileSubpassNode.hpp"

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Make.hpp>

#include "../../Component/Compile/CompileSubpassComponent.hpp"
#include "../../Component/Compile/Subpass/CompileSubpassAccelComponent.hpp"
#include "../../Component/Compile/Subpass/CompileSubpassMaterialAccelComponent.hpp"
#include "../../Component/Compile/Subpass/CompileSubpassMultiAccelComponent.hpp"
#include "../../Component/Compile/Subpass/CompileSubpassSingleAccelComponent.hpp"
#include "../../Visitor/Visitor.hpp"

using namespace hg::engine::gfx::render::graph;
using namespace hg;

CompileSubpassNode::CompileSubpassNode(SubpassAccelMode mode_) noexcept :
    CompileNode(
        make_smr<Auxiliary>()
    ) {
    auxAdd(make_uptr<CompileSubpassComponent>());

    if (mode_ == SubpassAccelMode::eSingle) {
        auxAdd(make_uptr<CompileSubpassSingleAccelComponent>());
    } else if (mode_ == SubpassAccelMode::eMulti) {
        auxAdd(make_uptr<CompileSubpassMultiAccelComponent>());
    } else if (mode_ == SubpassAccelMode::eMaterial) {
        auxAdd(make_uptr<CompileSubpassMaterialAccelComponent>());
    }
}

void CompileSubpassNode::accept(ref<Visitor> visitor_) const {
    visitor_(*this);
}

void CompileSubpassNode::traverse(ref<Visitor> visitor_) const {
    _next->accept(visitor_);
}

void CompileSubpassNode::setNext(mref<smr<Node>> next_) {
    _next = _STD move(next_);
}

smr<Node> CompileSubpassNode::getNext() const noexcept {
    return _next;
}

nmpt<CompileSubpassComponent> CompileSubpassNode::getSubpassComponent() const noexcept {
    const auto it = _STD ranges::find_if(
        _auxiliary->components(),
        [](const ptr<Component> component_) {
            return component_->getMetaClass()->exact<CompileSubpassComponent>();
        }
    );
    return it != _auxiliary->components().end() ?
               static_cast<ptr<CompileSubpassComponent>>(*it) :
               nmpt<CompileSubpassComponent> {};
}

nmpt<CompileSubpassAccelComponent> CompileSubpassNode::getSubpassAcceleration() const noexcept {
    const auto it = _STD ranges::find_if(
        _auxiliary->components(),
        [](const ptr<Component> component_) {
            return component_->getMetaClass()->is<CompileSubpassAccelComponent>();
        }
    );
    return it != _auxiliary->components().end() ?
               static_cast<ptr<CompileSubpassAccelComponent>>(*it) :
               nmpt<CompileSubpassAccelComponent> {};
}