#include "ExecutionPass.hpp"

using namespace hg::engine::render::graph;
using namespace hg;

ExecutionPass::VolatileExecutionVisitor::VolatileExecutionVisitor(const ptr<ExecutionPass> owner_) noexcept :
    _owner(owner_),
    _backlog() {}

ExecutionPass::VolatileExecutionVisitor::~VolatileExecutionVisitor() noexcept = default;

void ExecutionPass::VolatileExecutionVisitor::step() {

    const auto node = _backlog.front();

    [[maybe_unused]] const auto invalidation = static_cast<ptr<RuntimeNode>>(node.get())->execute(_owner->_context);

    node->traverse(*this);
    _backlog.pop();
}

void ExecutionPass::VolatileExecutionVisitor::unroll() {

    while (not _backlog.empty()) {
        step();
    }

}

void ExecutionPass::VolatileExecutionVisitor::operator()(cref<Node> node_) {

    const auto base = _backlog.empty();
    _backlog.emplace(&const_cast<ref<Node>>(node_));

    if (base) {
        unroll();
    }
}
