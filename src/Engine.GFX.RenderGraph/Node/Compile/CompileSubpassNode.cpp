#include "CompileSubPassNode.hpp"

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Make.hpp>

#include "../../Visitor/Visitor.hpp"

using namespace hg::engine::gfx::render::graph;
using namespace hg;

CompileSubPassNode::CompileSubPassNode() noexcept :
    CompileNode(
        make_smr<Auxiliary>()
    ) {}

void CompileSubPassNode::accept(ref<Visitor> visitor_) const {
    visitor_(*this);
}

void CompileSubPassNode::traverse(ref<Visitor> visitor_) const {
    _next->accept(visitor_);
}

void CompileSubPassNode::rtraverse(ref<Visitor> visitor_) const {
    _prev->accept(visitor_);
}

void CompileSubPassNode::setNext(mref<smr<Node>> next_) {
    _next = _STD move(next_);
}

smr<Node> CompileSubPassNode::getNext() const noexcept {
    return _next;
}

void CompileSubPassNode::setPrev(mref<nmpt<const Node>> prev_) {
    _prev = _STD move(prev_);
}

nmpt<const Node> CompileSubPassNode::getPrev() const noexcept {
    return _prev;
}

void CompileSubPassNode::setSubPassBuilder(mref<decltype(_subPassBuilder)> subPassBuilder_) noexcept {
    _subPassBuilder = _STD move(subPassBuilder_);
}

uptr<RuntimeNode> CompileSubPassNode::compile(cref<CompilePassContext> ctx_) const noexcept {
    assert(_subPassBuilder);
    return _subPassBuilder(ctx_);
}
