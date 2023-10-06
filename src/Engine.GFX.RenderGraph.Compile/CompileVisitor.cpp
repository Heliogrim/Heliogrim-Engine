#include "CompileVisitor.hpp"

#include <Engine.GFX.RenderGraph/RuntimeGraph.hpp>
#include <Engine.GFX.RenderGraph/Builder/Builder.hpp>
#include <Engine.GFX.RenderGraph/Component/Compile/Subpass/CompileSubpassAccelComponent.hpp>
#include <Engine.GFX.RenderGraph/Component/Compile/Subpass/CompileSubpassMaterialAccelComponent.hpp>
#include <Engine.GFX.RenderGraph/Component/Compile/Subpass/CompileSubpassMultiAccelComponent.hpp>
#include <Engine.GFX.RenderGraph/Component/Compile/Subpass/CompileSubpassSingleAccelComponent.hpp>
#include <Engine.GFX.RenderGraph/Component/Subpass/SubpassSceneMeshInvocationComponent.hpp>
#include <Engine.Reflect/TypeSwitch.hpp>

/**/
#include <Engine.GFX.RenderGraph/Node/Node.hpp>
#include <Engine.GFX.RenderGraph/Node/BarrierNode.hpp>
#include <Engine.GFX.RenderGraph/Node/ConvergeNode.hpp>
#include <Engine.GFX.RenderGraph/Node/DivergeNode.hpp>
#include <Engine.GFX.RenderGraph/Node/ProviderNode.hpp>
#include <Engine.GFX.RenderGraph/Node/SelectorNode.hpp>
#include <Engine.GFX.RenderGraph/Node/Compile/CompileSubpassNode.hpp>
/**/

using namespace hg::engine::gfx::render::graph;
using namespace hg::engine::gfx;
using namespace hg;

CompileVisitor::CompileVisitor(mref<nmpt<RuntimeGraph>> target_) noexcept :
    _target(_STD move(target_)) {}

void CompileVisitor::operator()(cref<Node> node_) {
    return node_.rtraverse(*this);
}

void CompileVisitor::operator()(cref<AnchorNode> node_) {

    auto allocated = make_smr<AnchorNode>();
    Builder::insertNode(
        _STD as_const(*_target).begin(),
        _STD as_const(*_target).end(),
        _STD move(allocated),
        _target
    );

    Visitor::operator()(node_);
}

void CompileVisitor::operator()(cref<BarrierNode> node_) {

    auto allocated = make_smr<BarrierNode>();
    Builder::insertNode(
        _STD as_const(*_target).begin(),
        _STD as_const(*_target).end(),
        _STD move(allocated),
        _target
    );

    Visitor::operator()(node_);
}

void CompileVisitor::operator()(cref<ConvergeNode> node_) {

    auto allocated = make_smr<ConvergeNode>();
    Builder::insertNode(
        _STD as_const(*_target).begin(),
        _STD as_const(*_target).end(),
        _STD move(allocated),
        _target
    );

    Visitor::operator()(node_);
}

void CompileVisitor::operator()(cref<DivergeNode> node_) {

    auto allocated = make_smr<DivergeNode>();
    Builder::insertNode(
        _STD as_const(*_target).begin(),
        _STD as_const(*_target).end(),
        _STD move(allocated),
        _target
    );

    Visitor::operator()(node_);
}

void CompileVisitor::operator()(cref<SelectorNode> node_) {

    auto allocated = make_smr<SelectorNode>();
    Builder::insertNode(
        _STD as_const(*_target).begin(),
        _STD as_const(*_target).end(),
        _STD move(allocated),
        _target
    );

    Visitor::operator()(node_);
}

void CompileVisitor::operator()(cref<ProviderNode> node_) {

    auto allocated = make_smr<ProviderNode>();
    Builder::insertNode(
        _STD as_const(*_target).begin(),
        _STD as_const(*_target).end(),
        _STD move(allocated),
        _target
    );

    Visitor::operator()(node_);
}

void CompileVisitor::operator()(cref<SubpassNode> node_) {
    assert(false);
}

void CompileVisitor::operator()(cref<CompileNode> node_) {
    Visitor::operator()(static_cast<cref<Node>>(node_));
}

void CompileVisitor::operator()(cref<CompileSubpassNode> node_) {

    const auto csc = node_.getSubpassComponent();
    const auto cac = node_.getSubpassAcceleration();

    auto sequence = switchType<
        CompileSubpassAccelComponent
    >(
        cac.get(),
        [](CompileSubpassSingleAccelComponent*) {
            // TODO:
            auto tmp = make_smr<SubpassNode>(graph::SubpassAccelMode::eSingle).into<Node>();
            return _STD make_pair(tmp, tmp);
        },
        [](CompileSubpassMultiAccelComponent*) {
            // TODO:
            auto tmp = make_smr<SubpassNode>(graph::SubpassAccelMode::eMulti).into<Node>();
            return _STD make_pair(tmp, tmp);
        },
        [](CompileSubpassMaterialAccelComponent*) {
            // TODO:
            auto tmp = make_smr<SubpassNode>(graph::SubpassAccelMode::eMaterial).into<Node>();
            return _STD make_pair(tmp, tmp);
        }
    );

    Builder::insertSubGraph(
        _STD as_const(*_target).begin(),
        _STD as_const(*_target).end(),
        _STD move(sequence.first),
        _STD move(sequence.second),
        _target
    );

    Visitor::operator()(node_);
}
