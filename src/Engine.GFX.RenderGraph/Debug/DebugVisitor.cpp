#include "DebugVisitor.hpp"

#include <Engine.Common/__macro.hpp>
#include <Engine.Logging/Logger.hpp>

#include "../Node/Node.hpp"
#include "../Node/Runtime/AnchorNode.hpp"
#include "../Node/Runtime/BarrierNode.hpp"
#include "../Node/CompileNode.hpp"

#include "../Component/AnchorComponent.hpp"
#include "../Component/BarrierComponent.hpp"

using namespace hg::engine::render::graph;
using namespace hg;

DebugVisitor::DebugVisitor() noexcept = default;

DebugVisitor::~DebugVisitor() noexcept = default;

void DebugVisitor::operator()(cref<Node> node_) {
    if (_seen.insert(&node_).second) {
        return node_.traverse(*this);
    }
    IM_DEBUG_LOG("<<(already seen)>>");
}

void DebugVisitor::operator()(cref<AnchorNode> node_) {

    const auto component = node_.getAnchorComponent();
    IM_DEBUG_LOGF(
        "AnchorNode `{}` `{:x}`",
        component->getAnchorName(),
        reinterpret_cast<ptrdiff_t>(std::addressof(node_))
    );

    Visitor::operator()(node_);
}

void DebugVisitor::operator()(cref<BarrierNode> node_) {

    const auto component = node_.getBarrierComponent();
    IM_DEBUG_LOGF(
        "BarrierNode `{}` `{:x}`",
        component->getBarrierName(),
        reinterpret_cast<ptrdiff_t>(std::addressof(node_))
    );

    Visitor::operator()(node_);
}

void DebugVisitor::operator()(cref<ConvergeNode> node_) {

    // TODO: Add tracking node marker for converging flow
    IM_DEBUG_LOGF("ConvergeNode `{:x}`", reinterpret_cast<ptrdiff_t>(std::addressof(node_)));

    Visitor::operator()(node_);
}

void DebugVisitor::operator()(cref<DivergeNode> node_) {

    // TODO: Add tracking node marker for diverging flow
    IM_DEBUG_LOGF("DivergeNode `{:x}`", reinterpret_cast<ptrdiff_t>(std::addressof(node_)));

    Visitor::operator()(node_);
}

void DebugVisitor::operator()(cref<SelectorNode> node_) {

    // TODO: Add tracking node marker for diverging flow
    IM_DEBUG_LOGF("SelectorNode `{:x}`", reinterpret_cast<ptrdiff_t>(std::addressof(node_)));

    Visitor::operator()(node_);
}

void DebugVisitor::operator()(cref<ProviderNode> node_) {

    IM_DEBUG_LOGF("ProviderNode `{:x}`", reinterpret_cast<ptrdiff_t>(std::addressof(node_)));

    Visitor::operator()(node_);
}

void DebugVisitor::operator()(cref<SubPassNodeBase> node_) {

    IM_DEBUG_LOGF("SubPassNodeBase `{:x}`", reinterpret_cast<ptrdiff_t>(std::addressof(node_)));

    Visitor::operator()(node_);
}

void DebugVisitor::operator()(cref<CompileNode> node_) {

    //IM_DEBUG_LOGF("CompileNode `{:x}`", reinterpret_cast<ptrdiff_t>(std::addressof(node_)));

    DebugVisitor::operator()(static_cast<cref<Node>>(node_));
}

void DebugVisitor::operator()(cref<CompileSubPassNode> node_) {

    IM_DEBUG_LOGF("CompileSubPassNode `{:x}`", reinterpret_cast<ptrdiff_t>(std::addressof(node_)));

    Visitor::operator()(node_);
}
