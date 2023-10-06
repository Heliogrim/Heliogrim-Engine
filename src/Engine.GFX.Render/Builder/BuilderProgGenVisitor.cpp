#include "BuilderProgGenVisitor.hpp"

#include <Engine.GFX.Acc.Compile/Spec/SimpleSpecificationStorage.hpp>
#include <Engine.GFX.RenderGraph/Node/AnchorNode.hpp>
#include <Engine.GFX.RenderGraph/Node/BarrierNode.hpp>
#include <Engine.GFX.RenderGraph/Node/ConvergeNode.hpp>
#include <Engine.GFX.RenderGraph/Node/DivergeNode.hpp>
#include <Engine.GFX.RenderGraph/Node/ProviderNode.hpp>
#include <Engine.GFX.RenderGraph/Node/SelectorNode.hpp>
#include <Engine.GFX.RenderGraph/Node/SubpassNode.hpp>
#include <Engine.GFX.RenderPipeline/Stage/SubpassStage.hpp>
#include <Engine.GFX.RenderPipeline/Stage/ConvergingStage.hpp>
#include <Engine.GFX.RenderPipeline/Stage/DivergingStage.hpp>
#include <Engine.GFX.RenderPipeline/Stage/InterlockStage.hpp>
#include <Engine.Reflect/Meta/TypeId.hpp>

using namespace hg::engine::gfx::acc;
using namespace hg::engine::gfx::render::graph;
using namespace hg::engine::gfx::render::pipeline;
using namespace hg::engine::gfx::render;
using namespace hg;

[[nodiscard]] smr<SubpassStage> genSubpassStage(cref<graph::SubpassNode> node_);

/**/

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

    auto tmp = make_smr<InterlockStage>();
    const auto rcDep = _nodeRcDep.find(&node_);

    if (const auto sigStage = recursiveDepLookup(*rcDep->second.begin()); not sigStage.empty()) {
        tmp->setExecStateCnd(&sigStage->getExecState().state);
    }

    /**/
    _mapping.insert(_STD make_pair(&node_, tmp.into<pipeline::Stage>()));
}

void BuilderProgGenVisitor::operator()(cref<graph::ConvergeNode> node_) {

    auto tmp = make_smr<ConvergingStage>();

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

    auto tmp = make_smr<DivergingStage>();
    const auto rcDep = _nodeRcDep.find(&node_);

    if (const auto sigStage = recursiveDepLookup(*rcDep->second.begin()); not sigStage.empty()) {
        tmp->setExecStateCnd(&sigStage->getExecState().state);
    }

    /**/
    _mapping.insert(_STD make_pair(&node_, tmp.into<pipeline::Stage>()));
}

void BuilderProgGenVisitor::operator()(cref<graph::SelectorNode> node_) {

    auto tmp = make_smr<DivergingStage>();
    const auto rcDep = _nodeRcDep.find(&node_);

    if (const auto sigStage = recursiveDepLookup(*rcDep->second.begin()); not sigStage.empty()) {
        tmp->setExecStateCnd(&sigStage->getExecState().state);
    }

    /**/
    _mapping.insert(_STD make_pair(&node_, tmp.into<pipeline::Stage>()));
}

void BuilderProgGenVisitor::operator()(cref<graph::ProviderNode> node_) {}

void BuilderProgGenVisitor::operator()(cref<graph::SubpassNode> node_) {

    auto tmp = genSubpassStage(node_);
    const auto rcDep = _nodeRcDep.find(&node_);

    if (const auto sigStage = recursiveDepLookup(*rcDep->second.begin()); not sigStage.empty()) {
        tmp->setExecStateCnd(&sigStage->getExecState().state);
    }

    /**/
    _mapping.insert(_STD make_pair(&node_, tmp.into<pipeline::Stage>()));
}

void BuilderProgGenVisitor::operator()(cref<graph::CompileNode> node_) {
    assert(false);
}

/**/

#include <Engine.GFX.RenderPipeline/Stage/AccelSubpassStage.hpp>
#include <Engine.GFX.RenderPipeline/Stage/MaterialSubpassStage.hpp>
#include <Engine.GFX.RenderPipeline/Stage/MultiAccelSubpassStage.hpp>
#include <Engine.GFX.RenderGraph/Component/SubpassComponent.hpp>
#include <Engine.GFX.RenderGraph/Component/Subpass/SubpassInvocationComponent.hpp>
#include <Engine.GFX.RenderGraph/Component/Subpass/SubpassSceneMeshInvocationComponent.hpp>
#include <Engine.GFX.RenderGraph/Component/Subpass/SubpassSingleAccelComponent.hpp>
#include <Engine.GFX.RenderGraph/Component/Subpass/SubpassMaterialAccelComponent.hpp>
#include <Engine.GFX.RenderGraph/Component/Subpass/SubpassMultiAccelComponent.hpp>
#include <Engine.GFX.RenderGraph/Relation/MeshDescription.hpp>
#include <Engine.GFX.RenderGraph/Relation/ModelDescription.hpp>
#include <Engine.GFX.RenderGraph/Relation/SceneViewDescription.hpp>
#include <Engine.GFX.RenderGraph/Relation/TextureDescription.hpp>

auto makeSimpleSpec(nmpt<SubpassComponent> sc_) {

    auto spec = make_smr<SimpleSpecificationStorage>();
    auto gspec = make_uptr<GraphicsPassSpecification>();

    gspec->depthCompareOp = DepthCompareOp::eGreaterEqual;
    gspec->stencilCompareOp = StencilCompareOp::eNever;

    gspec->primitiveTopology = PrimitiveTopology::eTriangleList;

    s64 inMountedIdx = -1LL;
    for (const auto& req : sc_->getRequirements()) {

        const auto inMeta = req.symbol->description->getMetaClass();
        switch (inMeta->typeId().data) {
            case reflect::typeId<MeshDescription>().data: {
                auto desc = static_cast<ptr<MeshDescription>>(req.symbol->description.get());
                break;
            }
            case reflect::typeId<ModelDescription>().data: {
                auto desc = static_cast<ptr<ModelDescription>>(req.symbol->description.get());
                break;
            }
            case reflect::typeId<SceneViewDescription>().data: {
                auto desc = static_cast<ptr<SceneViewDescription>>(req.symbol->description.get());
                break;
            }
            case reflect::typeId<TextureDescription>().data: {
                auto desc = static_cast<ptr<TextureDescription>>(req.symbol->description.get());
                break;
            }
        }

    }

    for (const auto& out : sc_->getProvided()) {

        const auto outMeta = out.symbol->description->getMetaClass();
        switch (outMeta->typeId().data) {
            case reflect::typeId<TextureDescription>().data: {
                gspec->requiredExports.push_back(clone(out.symbol));
            }
        }
    }

    return spec;
}

smr<SubpassStage> genAccelSubpassStage(
    nmpt<SubpassComponent> sc_,
    nmpt<SubpassSingleAccelComponent> ac_,
    nmpt<SubpassInvocationComponent> ic_
) {
    return make_smr<AccelSubpassStage>();
}

smr<SubpassStage> genMultiAccelSubpassStage(
    nmpt<SubpassComponent> sc_,
    nmpt<SubpassMultiAccelComponent> ac_,
    nmpt<SubpassInvocationComponent> ic_
) {
    return make_smr<MultiAccelSubpassStage>();
}

smr<SubpassStage> genMaterialSubpassStage(
    nmpt<SubpassComponent> sc_,
    nmpt<SubpassMaterialAccelComponent> ac_,
    nmpt<SubpassInvocationComponent> ic_
) {
    return make_smr<MaterialSubpassStage>();
}

smr<SubpassStage> genSubpassStage(cref<graph::SubpassNode> node_) {

    const auto mc = node_.getSubpassComponent();
    const auto ac = node_.getSubpassAcceleration();
    const auto ic = node_.getSubpassInvocation();

    switch (ac->getMetaClass()->typeId().data) {
        case reflect::typeId<graph::SubpassSingleAccelComponent>().data: return
                genAccelSubpassStage(mc, static_cast<ptr<SubpassSingleAccelComponent>>(ac.get()), ic);
        case reflect::typeId<graph::SubpassMultiAccelComponent>().data: return
                genMultiAccelSubpassStage(mc, static_cast<ptr<SubpassMultiAccelComponent>>(ac.get()), ic);
        case reflect::typeId<graph::SubpassMaterialAccelComponent>().data: return
                genMaterialSubpassStage(mc, static_cast<ptr<SubpassMaterialAccelComponent>>(ac.get()), ic);
        default: {
            return nullptr;
        }
    }
}
