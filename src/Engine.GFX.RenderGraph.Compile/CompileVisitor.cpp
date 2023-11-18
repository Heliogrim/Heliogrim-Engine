#include "CompileVisitor.hpp"

#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.GFX.RenderGraph/RuntimeGraph.hpp>
#include <Engine.GFX.RenderGraph/Builder/Builder.hpp>
#include <Engine.Reflect/TypeSwitch.hpp>

/**/
#include <Engine.GFX.Acc.Compile/VkEffectCompiler.hpp>
#include <Engine.GFX.Acc.Compile/Spec/SimpleEffectSpecification.hpp>
#include <Engine.GFX.RenderGraph/Node/CompileNode.hpp>
#include <Engine.GFX.RenderGraph/Node/Node.hpp>
#include <Engine.GFX.RenderGraph/Node/Compile/CompileSubPassNode.hpp>
#include <Engine.GFX.RenderGraph/Node/Runtime/AnchorNode.hpp>
#include <Engine.GFX.RenderGraph/Node/Runtime/BarrierNode.hpp>
#include <Engine.GFX.RenderGraph/Node/Runtime/ConvergeNode.hpp>
#include <Engine.GFX.RenderGraph/Node/Runtime/DivergeNode.hpp>
#include <Engine.GFX.RenderGraph/Node/Runtime/ProviderNode.hpp>
#include <Engine.GFX.RenderGraph/Node/Runtime/SelectorNode.hpp>
#include <Engine.GFX.RenderGraph/Pass/CompilePassContext.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>
#include <Engine.Reflect/Cast.hpp>
/**/

using namespace hg::engine::gfx::render::graph;
using namespace hg;

/**/

CompileVisitor::CompileVisitor(mref<nmpt<RuntimeGraph>> target_) noexcept :
    Visitor(),
    _target(_STD move(target_)) {}

void CompileVisitor::operator()(cref<Node> node_) {
    return node_.rtraverse(*this);
}

void CompileVisitor::operator()(cref<AnchorNode> node_) {

    auto allocated = make_smr<AnchorNode>();
    Builder::insertNode(
        _STD as_const(*_target).begin(),
        _target->begin()->getNext().get(),
        _STD move(allocated),
        _target
    );

    Visitor::operator()(node_);
}

void CompileVisitor::operator()(cref<BarrierNode> node_) {

    auto allocated = make_smr<BarrierNode>();
    Builder::insertNode(
        _STD as_const(*_target).begin(),
        _target->begin()->getNext().get(),
        _STD move(allocated),
        _target
    );

    Visitor::operator()(node_);
}

void CompileVisitor::operator()(cref<ConvergeNode> node_) {

    auto allocated = make_smr<ConvergeNode>();
    Builder::insertNode(
        _STD as_const(*_target).begin(),
        _target->begin()->getNext().get(),
        _STD move(allocated),
        _target
    );

    Visitor::operator()(node_);
}

void CompileVisitor::operator()(cref<DivergeNode> node_) {

    auto allocated = make_smr<DivergeNode>();
    Builder::insertNode(
        _STD as_const(*_target).begin(),
        _target->begin()->getNext().get(),
        _STD move(allocated),
        _target
    );

    Visitor::operator()(node_);
}

void CompileVisitor::operator()(cref<SelectorNode> node_) {

    auto allocated = make_smr<SelectorNode>();
    Builder::insertNode(
        _STD as_const(*_target).begin(),
        _target->begin()->getNext().get(),
        _STD move(allocated),
        _target
    );

    Visitor::operator()(node_);
}

void CompileVisitor::operator()(cref<ProviderNode> node_) {

    auto allocated = make_smr<ProviderNode>();
    Builder::insertNode(
        _STD as_const(*_target).begin(),
        _target->begin()->getNext().get(),
        _STD move(allocated),
        _target
    );

    Visitor::operator()(node_);
}

void CompileVisitor::operator()(cref<SubPassNodeBase> node_) {
    assert(false);
}

void CompileVisitor::operator()(cref<CompileNode> node_) {

    const CompilePassContext context {};
    auto compiled = node_.compile(context);

    if (compiled.operator bool()) {
        Builder::insertNode(
            _STD as_const(*_target).begin(),
            _target->begin()->getNext().get(),
            make_smr(_STD move(compiled)),
            _target
        );
    }

    this->operator()(static_cast<cref<Node>>(node_));
}
