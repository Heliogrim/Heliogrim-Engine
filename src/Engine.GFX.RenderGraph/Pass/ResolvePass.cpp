#include "ResolvePass.hpp"

using namespace hg::engine::render::graph;
using namespace hg;

ResolvePass::VolatileResolveVisitor::VolatileResolveVisitor(const ptr<ResolvePass> owner_) noexcept :
    _owner(owner_),
    _backlog() {}

ResolvePass::VolatileResolveVisitor::~VolatileResolveVisitor() noexcept = default;

void ResolvePass::VolatileResolveVisitor::step() {

    const auto node = _backlog.front();

    [[maybe_unused]] const auto invalidation = static_cast<ptr<RuntimeNode>>(node.get())->resolve(_owner->_context);

    node->traverse(*this);
    _backlog.pop();
}

void ResolvePass::VolatileResolveVisitor::unroll() {

    while (not _backlog.empty()) {
        step();
    }

}

void ResolvePass::VolatileResolveVisitor::operator()(cref<Node> node_) {

    const auto base = _backlog.empty();
    _backlog.emplace(&const_cast<ref<Node>>(node_));

    if (base) {
        unroll();
    }
}
