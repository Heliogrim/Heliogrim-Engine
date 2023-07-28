#include "BuilderProgGenVisitor.hpp"

#include <Engine.GFX.RenderGraph/Node/AnchorNode.hpp>
#include <Engine.GFX.RenderGraph/Node/BarrierNode.hpp>
#include <Engine.GFX.RenderGraph/Node/ConvergeNode.hpp>
#include <Engine.GFX.RenderGraph/Node/DivergeNode.hpp>
#include <Engine.GFX.RenderGraph/Node/ProviderNode.hpp>
#include <Engine.GFX.RenderGraph/Node/SelectorNode.hpp>
#include <Engine.GFX.RenderGraph/Node/SubpassNode.hpp>
#include <Engine.GFX.RenderPipeline/Stage/AccelSubpassStage.hpp>
#include <Engine.GFX.RenderPipeline/Stage/ConvergingStage.hpp>
#include <Engine.GFX.RenderPipeline/Stage/DivergingStage.hpp>
#include <Engine.GFX.RenderPipeline/Stage/InterlockStage.hpp>

using namespace hg::engine::gfx::render;
using namespace hg;

BuilderProgGenVisitor::BuilderProgGenVisitor() noexcept = default;

BuilderProgGenVisitor::~BuilderProgGenVisitor() = default;

smr<pipeline::Stage> BuilderProgGenVisitor::recursiveDepLookup(ptr<const graph::Node> initial_) {

    const auto sigStage = _mapping.find(initial_);
    if (sigStage != _mapping.end()) {
        return sigStage->second;
    }

    const auto deps = _nodeRcDep.find(initial_);
    if (deps == _nodeRcDep.end() || deps->second.empty()) {
        return nullptr;
    }

    return recursiveDepLookup(*deps->second.begin());
}

void BuilderProgGenVisitor::setNodeRcDep(mref<decltype(_nodeRcDep)> map_) {
    _nodeRcDep = _STD move(map_);
}

cref<decltype(BuilderProgGenVisitor::_mapping)> BuilderProgGenVisitor::getNodeStageMapping() {
    return _mapping;
}

decltype(BuilderProgGenVisitor::_mapping) BuilderProgGenVisitor::extractNodeStageMapping() {
    return _STD move(_mapping);
}

smr<pipeline::Stage> BuilderProgGenVisitor::findStage(ptr<const graph::Node> node_) {
    if (const auto it = _mapping.find(node_); it != _mapping.end()) {
        return clone(it->second);
    }

    return nullptr;
}

void BuilderProgGenVisitor::operator()(cref<graph::Node>) {
    assert(false);
}

void BuilderProgGenVisitor::operator()(cref<graph::AnchorNode> node_) {}

void BuilderProgGenVisitor::operator()(cref<graph::BarrierNode> node_) {

    auto tmp = make_smr<pipeline::InterlockStage>();
    const auto rcDep = _nodeRcDep.find(&node_);

    if (const auto sigStage = recursiveDepLookup(*rcDep->second.begin()); not sigStage.empty()) {
        tmp->setExecStateCnd(&sigStage->getExecState().state);
    }

    /**/
    _mapping.insert(_STD make_pair(&node_, tmp.into<pipeline::Stage>()));
}

void BuilderProgGenVisitor::operator()(cref<graph::ConvergeNode> node_) {

    auto tmp = make_smr<pipeline::ConvergingStage>();

    const auto rcDep = _nodeRcDep.find(&node_);
    for (const auto dep : rcDep->second) {

        const auto sigStage = recursiveDepLookup(dep);
        assert(not sigStage.empty());

        tmp->setExecStateCnd(&sigStage->getExecState().state);
    }

    /**/
    _mapping.insert(_STD make_pair(&node_, tmp.into<pipeline::Stage>()));
}

void BuilderProgGenVisitor::operator()(cref<graph::DivergeNode> node_) {

    auto tmp = make_smr<pipeline::DivergingStage>();
    const auto rcDep = _nodeRcDep.find(&node_);

    if (const auto sigStage = recursiveDepLookup(*rcDep->second.begin()); not sigStage.empty()) {
        tmp->setExecStateCnd(&sigStage->getExecState().state);
    }

    /**/
    _mapping.insert(_STD make_pair(&node_, tmp.into<pipeline::Stage>()));
}

void BuilderProgGenVisitor::operator()(cref<graph::SelectorNode> node_) {

    auto tmp = make_smr<pipeline::DivergingStage>();
    const auto rcDep = _nodeRcDep.find(&node_);

    if (const auto sigStage = recursiveDepLookup(*rcDep->second.begin()); not sigStage.empty()) {
        tmp->setExecStateCnd(&sigStage->getExecState().state);
    }

    /**/
    _mapping.insert(_STD make_pair(&node_, tmp.into<pipeline::Stage>()));
}

void BuilderProgGenVisitor::operator()(cref<graph::ProviderNode> node_) {}

void BuilderProgGenVisitor::operator()(cref<graph::SubpassNode> node_) {
    auto tmp = make_smr<pipeline::AccelSubpassStage>();
    const auto rcDep = _nodeRcDep.find(&node_);

    if (const auto sigStage = recursiveDepLookup(*rcDep->second.begin()); not sigStage.empty()) {
        tmp->setExecStateCnd(&sigStage->getExecState().state);
    }

    /**/
    _mapping.insert(_STD make_pair(&node_, tmp.into<pipeline::Stage>()));
}
