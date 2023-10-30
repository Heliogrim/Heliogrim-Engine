#include "ProgressiveGenerator.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.GFX.RenderGraph/Node/Runtime/BarrierNode.hpp>
#include <Engine.GFX.RenderGraph/Node/Runtime/ConvergeNode.hpp>
#include <Engine.GFX.RenderGraph/Node/Runtime/DivergeNode.hpp>
#include <Engine.GFX.RenderGraph/Node/Runtime/ProviderNode.hpp>
#include <Engine.GFX.RenderGraph/Node/Runtime/SelectorNode.hpp>
#include <Engine.GFX.RenderGraph/Node/Runtime/SubpassNode.hpp>
#include <Engine.Reflect/TypeSwitch.hpp>

#include "../Stage/AccelSubpassStage.hpp"
#include "../Stage/ConvergingStage.hpp"
#include "../Stage/DivergingStage.hpp"
#include "../Stage/InterlockStage.hpp"
#include "../Stage/MaterialSubpassStage.hpp"
#include "../Stage/MultiAccelSubpassStage.hpp"
#include "../Stage/SubpassStage.hpp"

using namespace hg::engine::gfx::render::pipeline;
using namespace hg::engine::gfx::render;
using namespace hg;

/**/

ProgressiveGenerator::ProgressiveGenerator() noexcept = default;

ProgressiveGenerator::~ProgressiveGenerator() noexcept = default;

void ProgressiveGenerator::storeKnownDependencies(mref<decltype(_knownDependencies)> dependencies_) {
    _knownDependencies = _STD move(dependencies_);
}

decltype(ProgressiveGenerator::_knownDependencies) ProgressiveGenerator::extractKnownDependencies() noexcept {
    return _STD move(_knownDependencies);
}

decltype(ProgressiveGenerator::_nodeToStage) ProgressiveGenerator::extractProgressiveMapping() noexcept {
    return _STD move(_nodeToStage);
}

smr<Stage> ProgressiveGenerator::findStage(const ptr<const graph::Node> node_) noexcept {
    const auto iter = _nodeToStage.find(node_);
    return iter != _nodeToStage.end() ? iter->second : smr<Stage> {};
}

void ProgressiveGenerator::operator()(cref<graph::Node>) {
    assert(false);
}

void ProgressiveGenerator::operator()(cref<graph::AnchorNode>) {
    /* __noop; */
}

void ProgressiveGenerator::operator()(cref<graph::BarrierNode> node_) {

    auto tmp = make_smr<InterlockStage>();
    _nodeToStage.insert(_STD make_pair(&node_, tmp.into<Stage>()));
}

void ProgressiveGenerator::operator()(cref<graph::ConvergeNode> node_) {

    auto tmp = make_smr<ConvergingStage>();
    _nodeToStage.insert(_STD make_pair(&node_, tmp.into<Stage>()));
}

void ProgressiveGenerator::operator()(cref<graph::DivergeNode> node_) {

    auto tmp = make_smr<DivergingStage>();
    _nodeToStage.insert(_STD make_pair(&node_, tmp.into<Stage>()));
}

void ProgressiveGenerator::operator()(cref<graph::SelectorNode> node_) {

    auto tmp = make_smr<DivergingStage>();
    _nodeToStage.insert(_STD make_pair(&node_, tmp.into<Stage>()));
}

void ProgressiveGenerator::operator()(cref<graph::ProviderNode> node_) {
    /* __noop; */
}

void ProgressiveGenerator::operator()(cref<graph::SubpassNode> node_) {

    assert(false);

    //auto tmp = make_smr<SubpassStage>();
    //_nodeToStage.insert(_STD make_pair(&node_, tmp.into<Stage>()));
}

void ProgressiveGenerator::operator()(cref<graph::CompileNode> node_) {
    assert(false);
}

/**/
#if FALSE
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
#endif
