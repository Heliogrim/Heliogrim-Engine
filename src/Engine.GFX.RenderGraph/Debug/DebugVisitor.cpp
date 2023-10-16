#include "DebugVisitor.hpp"

#include <Engine.Common/__macro.hpp>
#include <Engine.Logging/Logger.hpp>
#include <Engine.Reflect/TypeSwitch.hpp>

#include "../Node/Node.hpp"
#include "../Node/AnchorNode.hpp"
#include "../Node/BarrierNode.hpp"
#include "../Node/Compile/CompileNode.hpp"

#include "../Component/AnchorComponent.hpp"
#include "../Component/BarrierComponent.hpp"
#include "../Component/Subpass/SubpassMaterialAccelComponent.hpp"
#include "../Component/Subpass/SubpassMultiAccelComponent.hpp"
#include "../Component/Subpass/SubpassSingleAccelComponent.hpp"
#include "Engine.GFX.RenderGraph/Node/SubpassNode.hpp"

using namespace hg::engine::gfx::render::graph;
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
        reinterpret_cast<ptrdiff_t>(_STD addressof(node_))
    );

    Visitor::operator()(node_);
}

void DebugVisitor::operator()(cref<BarrierNode> node_) {

    const auto component = node_.getBarrierComponent();
    IM_DEBUG_LOGF(
        "BarrierNode `{}` `{:x}`",
        component->getBarrierName(),
        reinterpret_cast<ptrdiff_t>(_STD addressof(node_))
    );

    Visitor::operator()(node_);
}

void DebugVisitor::operator()(cref<ConvergeNode> node_) {

    // TODO: Add tracking node marker for converging flow
    IM_DEBUG_LOGF("ConvergeNode `{:x}`", reinterpret_cast<ptrdiff_t>(_STD addressof(node_)));

    Visitor::operator()(node_);
}

void DebugVisitor::operator()(cref<DivergeNode> node_) {

    // TODO: Add tracking node marker for diverging flow
    IM_DEBUG_LOGF("DivergeNode `{:x}`", reinterpret_cast<ptrdiff_t>(_STD addressof(node_)));

    Visitor::operator()(node_);
}

void DebugVisitor::operator()(cref<SelectorNode> node_) {

    // TODO: Add tracking node marker for diverging flow
    IM_DEBUG_LOGF("SelectorNode `{:x}`", reinterpret_cast<ptrdiff_t>(_STD addressof(node_)));

    Visitor::operator()(node_);
}

void DebugVisitor::operator()(cref<ProviderNode> node_) {

    IM_DEBUG_LOGF("ProviderNode `{:x}`", reinterpret_cast<ptrdiff_t>(_STD addressof(node_)));

    Visitor::operator()(node_);
}

void DebugVisitor::operator()(cref<SubpassNode> node_) {

    const auto accel = switchType(
        node_.getSubpassAcceleration().get(),
        [](SubpassSingleAccelComponent*) {
            return "SubpassSingleAccelComponent";
        },
        [](SubpassMultiAccelComponent*) {
            return "SubpassMultiAccelComponent";
        },
        [](SubpassMaterialAccelComponent*) {
            return "SubpassMaterialAccelComponent";
        }
    );

    IM_DEBUG_LOGF("SubpassNode `{:x}` [{:s}]", reinterpret_cast<ptrdiff_t>(_STD addressof(node_)), accel);

    Visitor::operator()(node_);
}

void DebugVisitor::operator()(cref<CompileNode> node_) {

    //IM_DEBUG_LOGF("CompileNode `{:x}`", reinterpret_cast<ptrdiff_t>(_STD addressof(node_)));

    DebugVisitor::operator()(static_cast<cref<Node>>(node_));
}

void DebugVisitor::operator()(cref<CompileSubpassNode> node_) {

    IM_DEBUG_LOGF("CompileSubpassNode `{:x}`", reinterpret_cast<ptrdiff_t>(_STD addressof(node_)));

    Visitor::operator()(node_);
}
