#include "Visitor.hpp"

using namespace hg::engine::gfx::render::graph;
using namespace hg;

template <typename TargetType_, typename SourceType_>
constexpr cref<TargetType_> override_cast(cref<SourceType_> val_) noexcept {
    return *reinterpret_cast<const TargetType_*>(reinterpret_cast<const void*>(&val_));
}

void Visitor::operator()(cref<AnchorNode> node_) {
    (*this)(override_cast<Node>(node_));
}

void Visitor::operator()(cref<BarrierNode> node_) {
    (*this)(override_cast<Node>(node_));
}

void Visitor::operator()(cref<ConvergeNode> node_) {
    (*this)(override_cast<Node>(node_));
}

void Visitor::operator()(cref<DivergeNode> node_) {
    (*this)(override_cast<Node>(node_));
}

void Visitor::operator()(cref<SelectorNode> node_) {
    (*this)(override_cast<Node>(node_));
}

void Visitor::operator()(cref<ProviderNode> node_) {
    (*this)(override_cast<Node>(node_));
}

void Visitor::operator()(cref<SLNode> node_) {
    (*this)(override_cast<Node>(node_));
}

void Visitor::operator()(cref<SubPassNodeBase> node_) {
    (*this)(override_cast<SLNode>(node_));
}

void Visitor::operator()(cref<CompileSubPassNode> node_) {
    (*this)(override_cast<CompileNode>(node_));
}
