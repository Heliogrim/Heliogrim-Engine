#include "CompileVisitor.hpp"

#include <cassert>
#include <Engine.GFX.RenderGraph/RuntimeGraph.hpp>
#include <Engine.GFX.RenderGraph/Builder/Builder.hpp>
#include <Engine.GFX.RenderGraph/Component/AnchorComponent.hpp>
#include <Engine.GFX.RenderGraph/Component/BarrierComponent.hpp>
#include <Engine.GFX.RenderGraph/Component/Compile/Subpass/CompileSubpassAccelComponent.hpp>
#include <Engine.GFX.RenderGraph/Component/Compile/Subpass/CompileSubpassMaterialAccelComponent.hpp>
#include <Engine.GFX.RenderGraph/Component/Compile/Subpass/CompileSubpassMultiAccelComponent.hpp>
#include <Engine.GFX.RenderGraph/Component/Compile/Subpass/CompileSubpassSingleAccelComponent.hpp>
#include <Engine.GFX.RenderGraph/Component/Subpass/SubpassSceneMeshInvocationComponent.hpp>
#include <Engine.GFX.RenderGraph/Node/BarrierNode.hpp>
#include <Engine.GFX.RenderGraph/Node/ConvergeNode.hpp>
#include <Engine.GFX.RenderGraph/Node/DivergeNode.hpp>
#include <Engine.GFX.RenderGraph/Node/Node.hpp>
#include <Engine.GFX.RenderGraph/Node/ProviderNode.hpp>
#include <Engine.GFX.RenderGraph/Node/SelectorNode.hpp>
#include <Engine.GFX.RenderGraph/Node/Compile/CompileSubpassNode.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>
#include <Engine.Reflect/Meta/TypeId.hpp>

using namespace hg::engine::gfx::render;
using namespace hg;

CompileVisitor::CompileVisitor(
    mref<nmpt<graph::RuntimeGraph>> target_,
    mref<mapping_type> mapping_
) noexcept :
    _target(_STD move(target_)),
    _mapping(_STD move(mapping_)),
    _scope() {}

CompileVisitor::~CompileVisitor() = default;

void CompileVisitor::streamUnwind() {

    while (not _backlog.empty()) {

        auto next = _backlog.front();

        if (not _mapping.contains(next)) {
            __debugbreak();
        }

        _scope = next;
        next->traverse(*this);

        _backlog.pop();
    }
}

void CompileVisitor::operator()(cref<graph::Node> node_) {

    const auto root = _backlog.empty();
    _backlog.push(&node_);

    if (not root) {
        return;
    }

    streamUnwind();
}

void CompileVisitor::operator()(cref<graph::AnchorNode> node_) {

    const auto srcNext = node_.getNext();
    const auto storedIt = _mapping.find(srcNext.get());

    const nmpt<const graph::Node> seqBegin = _scope != nullptr ? _mapping.at(_scope).second : _target->begin().get();
    const nmpt<const graph::Node> seqEnd = storedIt != _mapping.end() ? storedIt->second.first : _target->end().get();

    /**/

    auto nextNode = make_smr<graph::AnchorNode>();

    // TODO: Transfer Attributes
    //nextNode->getAnchorComponent()->getAnchorName();

    _mapping.insert(_STD make_pair(&node_, _STD make_pair(nextNode.get(), nextNode.get())));
    graph::Builder::insertNode(seqBegin, seqEnd, _STD move(nextNode), _target);

    Visitor::operator()(node_);
}

void CompileVisitor::operator()(cref<graph::BarrierNode> node_) {

    const auto srcNext = node_.getNext();
    const auto storedIt = _mapping.find(srcNext.get());

    const nmpt<const graph::Node> seqBegin = _scope != nullptr ? _mapping.at(_scope).second : _target->begin().get();
    const nmpt<const graph::Node> seqEnd = storedIt != _mapping.end() ? storedIt->second.first : _target->end().get();

    /**/

    auto nextNode = make_smr<graph::BarrierNode>();

    // TODO: Transfer Attributes
    nextNode->getBarrierComponent()->setBarrierName(node_.getBarrierComponent()->getBarrierName());

    _mapping.insert(_STD make_pair(&node_, _STD make_pair(nextNode.get(), nextNode.get())));
    graph::Builder::insertNode(seqBegin, seqEnd, _STD move(nextNode), _target);

    Visitor::operator()(node_);
}

void CompileVisitor::operator()(cref<graph::ConvergeNode> node_) {

    const auto srcNext = node_.getNext();
    const auto storedIt = _mapping.find(srcNext.get());

    const nmpt<const graph::Node> seqBegin = _scope != nullptr ? _mapping.at(_scope).second : _target->begin().get();
    const nmpt<const graph::Node> seqEnd = storedIt != _mapping.end() ? storedIt->second.first : _target->end().get();

    /**/

    auto nextNode = make_smr<graph::ConvergeNode>();

    // TODO: Transfer Attributes

    _mapping.insert(_STD make_pair(&node_, _STD make_pair(nextNode.get(), nextNode.get())));
    graph::Builder::insertNode(seqBegin, seqEnd, _STD move(nextNode), _target);

    Visitor::operator()(node_);
}

void CompileVisitor::operator()(cref<graph::DivergeNode> node_) {

    const auto& srcNext = node_.getNext();

    auto storedIt = _mapping.end();
    for (const auto& srcNextNode : srcNext) {
        if ((storedIt = _mapping.find(srcNextNode.get())) != _mapping.end()) {
            break;
        }
    }

    const nmpt<const graph::Node> seqBegin = _scope != nullptr ? _mapping.at(_scope).second : _target->begin().get();
    const nmpt<const graph::Node> seqEnd = storedIt != _mapping.end() ? storedIt->second.first : _target->end().get();

    /**/

    auto nextNode = make_smr<graph::DivergeNode>();

    // TODO: Transfer Attributes

    _mapping.insert(_STD make_pair(&node_, _STD make_pair(nextNode.get(), nextNode.get())));
    graph::Builder::insertNode(seqBegin, seqEnd, _STD move(nextNode), _target);

    Visitor::operator()(node_);
}

void CompileVisitor::operator()(cref<graph::SelectorNode> node_) {

    const auto& srcNext = node_.getNext();

    auto storedIt = _mapping.end();
    for (const auto& srcNextNode : srcNext) {
        if ((storedIt = _mapping.find(srcNextNode.node.get())) != _mapping.end()) {
            break;
        }
    }

    const nmpt<const graph::Node> seqBegin = _scope != nullptr ? _mapping.at(_scope).second : _target->begin().get();
    const nmpt<const graph::Node> seqEnd = storedIt != _mapping.end() ? storedIt->second.first : _target->end().get();

    /**/

    auto nextNode = make_smr<graph::SelectorNode>();

    // TODO: Transfer Attributes

    _mapping.insert(_STD make_pair(&node_, _STD make_pair(nextNode.get(), nextNode.get())));
    graph::Builder::insertNode(seqBegin, seqEnd, _STD move(nextNode), _target);

    Visitor::operator()(node_);
}

void CompileVisitor::operator()(cref<graph::ProviderNode> node_) {

    const auto srcNext = node_.getNext();
    const auto storedIt = _mapping.find(srcNext.get());

    const nmpt<const graph::Node> seqBegin = _scope != nullptr ? _mapping.at(_scope).second : _target->begin().get();
    const nmpt<const graph::Node> seqEnd = storedIt != _mapping.end() ? storedIt->second.first : _target->end().get();

    /**/

    auto nextNode = make_smr<graph::ProviderNode>();

    // TODO: Transfer Attributes

    _mapping.insert(_STD make_pair(&node_, _STD make_pair(nextNode.get(), nextNode.get())));
    graph::Builder::insertNode(seqBegin, seqEnd, _STD move(nextNode), _target);

    Visitor::operator()(node_);

}

void CompileVisitor::operator()(cref<graph::SubpassNode> node_) {
    assert(false);
}

void CompileVisitor::operator()(cref<graph::CompileNode> node_) {
    CompileVisitor::operator()(static_cast<cref<graph::Node>>(node_));
}

/**/

_STD pair<smr<graph::Node>, smr<graph::Node>> CompileVisitor::constructSingleAccel(
    nmpt<graph::CompileSubpassComponent> csc_,
    nmpt<graph::CompileSubpassSingleAccelComponent> cac_
) {

    auto tmp = make_smr<graph::SubpassNode>(graph::SubpassAccelMode::eSingle);
    tmp->setSubpassInvocation(make_uptr<graph::SubpassSceneMeshInvocationComponent>());
    return _STD make_pair(tmp, tmp);
}

_STD pair<smr<graph::Node>, smr<graph::Node>> CompileVisitor::constructMultiAccel(
    nmpt<graph::CompileSubpassComponent> csc_,
    nmpt<graph::CompileSubpassMultiAccelComponent> cac_
) {

    auto tmp = make_smr<graph::SubpassNode>(graph::SubpassAccelMode::eMulti);
    tmp->setSubpassInvocation(make_uptr<graph::SubpassSceneMeshInvocationComponent>());
    return _STD make_pair(tmp, tmp);
}

_STD pair<smr<graph::Node>, smr<graph::Node>> CompileVisitor::constructMaterialAccel(
    nmpt<graph::CompileSubpassComponent> csc_,
    nmpt<graph::CompileSubpassMaterialAccelComponent> cac_
) {

    if (cac_->getMaterials().empty()) {
        return _STD make_pair(nullptr, nullptr);
    }

    auto seqBegin = make_smr<graph::DivergeNode>();
    auto seqEnd = make_smr<graph::ConvergeNode>();

    for (const auto& material : cac_->getMaterials()) {

        // Instantiate one node per material instance
        auto node = make_smr<graph::SubpassNode>(graph::SubpassAccelMode::eMaterial);
        node->setSubpassInvocation(make_uptr<graph::SubpassSceneMeshInvocationComponent>());

        node->setNext(clone(seqEnd));
        seqBegin->addNext(_STD move(node));
    }

    return _STD make_pair(_STD move(seqBegin), _STD move(seqEnd));
}

void CompileVisitor::operator()(cref<graph::CompileSubpassNode> node_) {

    const auto csc = node_.getSubpassComponent();
    const auto cac = node_.getSubpassAcceleration();

    _STD pair<smr<graph::Node>, smr<graph::Node>> subgraph {};
    switch (cac->getMetaClass()->typeId().data) {
        case reflect::typeId<graph::CompileSubpassSingleAccelComponent>().data: {
            subgraph = constructSingleAccel(
                csc,
                static_cast<ptr<graph::CompileSubpassSingleAccelComponent>>(cac.get())
            );
            break;
        }
        case reflect::typeId<graph::CompileSubpassMultiAccelComponent>().data: {
            subgraph = constructMultiAccel(
                csc,
                static_cast<ptr<graph::CompileSubpassMultiAccelComponent>>(cac.get())
            );
            break;
        }
        case reflect::typeId<graph::CompileSubpassMaterialAccelComponent>().data: {
            subgraph = constructMaterialAccel(
                csc,
                static_cast<ptr<graph::CompileSubpassMaterialAccelComponent>>(cac.get())
            );
            break;
        }
    }

    /**/

    const auto srcNext = node_.getNext();
    const auto storedIt = _mapping.find(srcNext.get());

    const nmpt<const graph::Node> seqBegin = _scope != nullptr ? _mapping.at(_scope).second : _target->begin().get();
    const nmpt<const graph::Node> seqEnd = storedIt != _mapping.end() ? storedIt->second.first : _target->end().get();

    /**/

    // TODO: Transfer Attributes

    _mapping.insert(_STD make_pair(&node_, _STD make_pair(subgraph.first.get(), subgraph.second.get())));
    graph::Builder::insertSubGraph(
        seqBegin,
        seqEnd,
        _STD move(subgraph.first),
        _STD move(subgraph.second),
        _target
    );

    Visitor::operator()(node_);
}
