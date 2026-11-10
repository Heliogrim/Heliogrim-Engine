#include "IterationPass.hpp"

using namespace hg::engine::render::graph;
using namespace hg;

IterationPass::VolatileIterationVisitor::VolatileIterationVisitor(const ptr<IterationPass> owner_) noexcept :
    _owner(owner_),
    _backlog() {}

IterationPass::VolatileIterationVisitor::~VolatileIterationVisitor() noexcept = default;

void IterationPass::VolatileIterationVisitor::step() {

    const auto node = _backlog.front();

    [[maybe_unused]] const auto invalidation = static_cast<ptr<RuntimeNode>>(node.get())->iterate(_owner->_context);

    node->traverse(*this);
    _backlog.pop();
}

void IterationPass::VolatileIterationVisitor::unroll() {

    while (not _backlog.empty()) {
        step();
    }

}

void IterationPass::VolatileIterationVisitor::operator()(cref<Node> node_) {

    const auto base = _backlog.empty();
    _backlog.emplace(&const_cast<ref<Node>>(node_));

    if (base) {
        unroll();
    }
}
