#include "ProgressiveGenerator.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.GFX.RenderGraph/Component/Subpass/SubpassMaterialAccelComponent.hpp>
#include <Engine.GFX.RenderGraph/Component/Subpass/SubpassMultiAccelComponent.hpp>
#include <Engine.GFX.RenderGraph/Component/Subpass/SubpassSingleAccelComponent.hpp>
#include <Engine.GFX.RenderGraph/Node/BarrierNode.hpp>
#include <Engine.GFX.RenderGraph/Node/ConvergeNode.hpp>
#include <Engine.GFX.RenderGraph/Node/DivergeNode.hpp>
#include <Engine.GFX.RenderGraph/Node/ProviderNode.hpp>
#include <Engine.GFX.RenderGraph/Node/SelectorNode.hpp>
#include <Engine.GFX.RenderGraph/Node/SubpassNode.hpp>
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

[[nodiscard]] static smr<SubpassStage> makeSubpassStage(cref<graph::SubpassNode> node_);

static smr<SubpassStage> buildAccelSubpassStage(cref<graph::SubpassNode> node_);

static smr<SubpassStage> buildMultiAccelSubpassStage(cref<graph::SubpassNode> node_);

static smr<SubpassStage> buildMaterialSubpassStage(cref<graph::SubpassNode> node_);

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

    auto tmp = makeSubpassStage(node_);
    _nodeToStage.insert(_STD make_pair(&node_, tmp.into<Stage>()));
}

void ProgressiveGenerator::operator()(cref<graph::CompileNode> node_) {
    assert(false);
}

/**/

smr<SubpassStage> makeSubpassStage(cref<graph::SubpassNode> node_) {

    auto accel = node_.getSubpassAcceleration();

    return switchType(
        accel.get(),
        [&node_](graph::SubpassSingleAccelComponent*) {
            return buildAccelSubpassStage(node_);
        },
        [&node_](graph::SubpassMultiAccelComponent*) {
            return buildMultiAccelSubpassStage(node_);
        },
        [&node_](graph::SubpassMaterialAccelComponent*) {
            return buildMaterialSubpassStage(node_);
        }
    );
}

smr<SubpassStage> buildAccelSubpassStage(cref<graph::SubpassNode> node_) {

    using StagedAccelPass = AccelSubpassStage::StagedAccelPass;

    const auto main = node_.getSubpassComponent();
    const auto accel = static_cast<graph::SubpassSingleAccelComponent*>(node_.getSubpassAcceleration().get());
    const auto invoke = node_.getSubpassInvocation();

    StagedAccelPass tmp {};

    return make_smr<AccelSubpassStage>(_STD move(tmp)).into<SubpassStage>();
}

smr<SubpassStage> buildMultiAccelSubpassStage(cref<graph::SubpassNode> node_) {

    using StagedAccelPass = MultiAccelSubpassStage::StagedAccelPass;

    const auto main = node_.getSubpassAcceleration();
    const auto accel = static_cast<graph::SubpassMultiAccelComponent*>(node_.getSubpassAcceleration().get());
    const auto invoke = node_.getSubpassInvocation();

    Vector<StagedAccelPass> tmp {};

    return make_smr<MultiAccelSubpassStage>(_STD move(tmp)).into<SubpassStage>();
}

smr<SubpassStage> buildMaterialSubpassStage(cref<graph::SubpassNode> node_) {

    using StagedAccelPass = MaterialSubpassStage::StagedAccelPass;

    const auto main = node_.getSubpassAcceleration();
    const auto accel = static_cast<graph::SubpassMaterialAccelComponent*>(node_.getSubpassAcceleration().get());
    const auto invoke = node_.getSubpassInvocation();

    DenseMap<smr<const engine::gfx::MaterialResource>, Vector<StagedAccelPass>> tmp {};

    return make_smr<MaterialSubpassStage>(_STD move(tmp)).into<SubpassStage>();
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
