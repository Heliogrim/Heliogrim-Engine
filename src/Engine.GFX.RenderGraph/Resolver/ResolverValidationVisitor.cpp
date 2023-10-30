#include "ResolverValidationVisitor.hpp"

#include <Engine.Common/__macro.hpp>
#include <Engine.Logging/Logger.hpp>
#include <Engine.GFX.Acc/Symbol/Symbol.hpp>

#include "../Node/Node.hpp"
#include "../Node/Runtime/ProviderNode.hpp"
#include "../Node/Runtime/SubpassNode.hpp"
#include "../Component/ProviderComponent.hpp"

using namespace hg::engine::gfx::render::graph;
using namespace hg::engine::gfx::render;
using namespace hg;

ResolverValidationVisitor::ResolverValidationVisitor() noexcept = default;

ResolverValidationVisitor::~ResolverValidationVisitor() = default;

void ResolverValidationVisitor::operator()(cref<Node> node_) {
    ResolverVisitor::operator()(node_);
}

void ResolverValidationVisitor::operator()(cref<AnchorNode> node_) {
    Visitor::operator()(node_);
}

void ResolverValidationVisitor::operator()(cref<BarrierNode> node_) {
    Visitor::operator()(node_);
}

void ResolverValidationVisitor::operator()(cref<ConvergeNode> node_) {
    Visitor::operator()(node_);
}

void ResolverValidationVisitor::operator()(cref<DivergeNode> node_) {
    Visitor::operator()(node_);
}

void ResolverValidationVisitor::operator()(cref<SelectorNode> node_) {
    Visitor::operator()(node_);
}

void ResolverValidationVisitor::operator()(cref<ProviderNode> node_) {

    const auto component = node_.getProviderComponent();
    _providers.push_back(component->getProvided());

    Visitor::operator()(node_);
}

void ResolverValidationVisitor::operator()(cref<SubpassNode> node_) {

    /*
    // TODO:
    const auto component = node_.getSubpassComponent();
    const auto& requirements = component->getRequirements();

    for (const auto& required : requirements) {

        const auto reqToProv = Provision { required.symbol };
        bool matched = false;

        for (const auto& provider : _providers) {
            if (provider.contains(reqToProv)) {
                matched = true;
                break;
            }
        }

        if (matched) {
            continue;
        }

        IM_CORE_WARNF(
            "Encountered unmet requirement `[{}]\{\}` for SubpassNode `{:x}` while in validation pass.",
            required.symbol->name,
            reinterpret_cast<ptrdiff_t>(required.symbol.get()),
            reinterpret_cast<ptrdiff_t>(_STD addressof(node_))
        );
    }
    */

    /**/

    // TODO: _providers.push_back(component->getProvided());
    // TODO: _providers.push_back(component->getProvided());

    Visitor::operator()(node_);
}

void ResolverValidationVisitor::operator()(cref<CompileNode> node_) {

    IM_CORE_ERRORF(
        "Encountered compile node `{}` within runtime graph while in validation pass.",
        reinterpret_cast<ptrdiff_t>(_STD addressof(node_))
    );

    ResolverVisitor::operator()(node_);
}

void ResolverValidationVisitor::operator()(cref<CompileSubpassNode> node_) {
    Visitor::operator()(node_);
}
