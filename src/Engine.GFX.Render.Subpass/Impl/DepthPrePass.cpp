#include "DepthPrePass.hpp"

#include <Engine.Core/Engine.hpp>
#include <Engine.Driver.Vulkan/VkRCmdTranslator.hpp>
#include <Engine.GFX.Render.Command/RenderCommandBuffer.hpp>
#include <Engine.GFX/Graphics.hpp>
#include <Engine.GFX.Render.Graph.Scene/Hook/ModelType.hpp>
#include <Engine.GFX.Render.Predefined/Symbols/SceneDepth.hpp>
#include <Engine.GFX.Render.Predefined/Symbols/SceneView.hpp>
#include <Engine.GFX.RenderGraph/Relation/SceneViewDescription.hpp>
#include <Engine.GFX.RenderGraph/Relation/TextureDescription.hpp>
#include <Engine.GFX.RenderGraph/Symbol/ScopedSymbolContext.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>
#include <Engine.Accel.Pipeline/AccelerationPipeline.hpp>
#include <Engine.Accel.Compile/Profile/EffectProfile.hpp>
#include <Engine.Accel.Compile/Spec/SimpleEffectSpecification.hpp>
#include <Engine.Accel.Pass/VkAccelerationPassFactory.hpp>
#include <Engine.Accel.Pipeline/GraphicsPipeline.hpp>
#include <Engine.Accel.Effect/Stage/StageFlags.hpp>
#include <Engine.Accel.Effect/Stage/TransferType.hpp>
#include <Engine.Accel.Compile/VkEffectCompiler.hpp>
#include <Engine.Accel.Lang/Intermediate.hpp>
#include <Engine.Reflect/Cast.hpp>
#include <Engine.Accel.Effect/AccelerationEffect.hpp>

using namespace hg::engine::render;
using namespace hg::engine::accel;
using namespace hg::engine::gfx;
using namespace hg;

[[nodiscard]] static smr<AccelerationEffect> build_test_effect();

[[nodiscard]] static smr<const GraphicsPass> build_test_pass();

[[nodiscard]] static EffectCompileResult build_test_pipeline(
    mref<smr<const AccelerationEffect>> effect_,
    mref<smr<const GraphicsPass>> pass_
);

/**/

void DepthPrePass::destroy() noexcept {
    MeshSubPass::destroy();

    auto device = Engine::getEngine()->getGraphics()->getCurrentDevice();

    device->vkDevice().destroySemaphore(_STD exchange(_tmpSemaphore, nullptr));

    _framebuffer->destroy();
    _framebuffer.reset();

    _compiled.pipeline.reset();
    _pass.reset();
    _effect.reset();
}

void DepthPrePass::declareInputs(ref<graph::ScopedSymbolContext> symCtx_) noexcept {
    symCtx_.registerImportSymbol(makeSceneViewSymbol(), &_resources.inSceneView);
}

void DepthPrePass::declareOutputs(ref<graph::ScopedSymbolContext> symCtx_) noexcept {
    symCtx_.registerExportSymbol(makeSceneDepthSymbol(), &_resources.outSceneDepth);
}

void DepthPrePass::iterate(cref<graph::ScopedSymbolContext> symCtx_) noexcept {

    /*
    assert(not _resources.inSceneView.empty());
    auto lSceneView = _resources.inSceneView->load<smr<const scene::SceneView>, graph::SceneViewDescription>();

    {
        graph::SceneWalker walker {};
        Vector<ptr<const MetaClass>> typeList = { GeometryModel::getStaticMetaClass() };
        walker.addHookFilter(make_uptr<graph::ModelTypeFilter>(_STD move(typeList)));
        walker.setHook(
            [ci = createCaptureObject()](const auto* const model_) {
                static_cast<const ptr<const GeometryModel>>(model_)->capture(ci);
            }
        );

        walker(*lSceneView);
    }
     */

    /**/

    if (_effect.empty()) {
        _effect = build_test_effect();
    }

    if (_pass.empty()) {
        _pass = build_test_pass();
    }
}

void DepthPrePass::resolve() noexcept {
    if (_effect && _compiled.flag == EffectCompileResultFlag::eUnknown) {
        _compiled = build_test_pipeline(clone(_effect), clone(_pass));
    }
}

void DepthPrePass::execute(cref<graph::ScopedSymbolContext> symCtx_) noexcept {

    assert(_effect);
    assert(_compiled.pipeline);

    /**/

    //auto lSceneDepth = _resources.outSceneDepth->load<smr<TextureLikeObject>, graph::TextureDescription>();
    auto rSceneDepth = symCtx_.getExportSymbol(makeSceneDepthSymbol());
    auto lrSceneDepth = rSceneDepth->load<smr<TextureLikeObject>, graph::TextureDescription>();

    if (!_framebuffer.empty() && _framebuffer->attachments().front() != lrSceneDepth) {
        _framebuffer->device()->vkDevice().destroySemaphore(_tmpSemaphore);
        _framebuffer->destroy();
        _framebuffer.reset();
    }

    if (_framebuffer.empty()) {

        const Vector<smr<const graph::Symbol>> outputSymbols { makeSceneDepthSymbol() };
        _framebuffer = make_smr<Framebuffer>(Engine::getEngine()->getGraphics()->getCurrentDevice());

        for (const auto& output : outputSymbols) {

            auto data = symCtx_.getExportSymbol(clone(output));

            const auto& texture = data->load<smr<TextureLikeObject>>();
            _framebuffer->addAttachment(clone(texture));
        }

        {
            const auto sceneDepthExtent = Cast<Texture>(lrSceneDepth.get())->extent();
            _framebuffer->setExtent(sceneDepthExtent);
        }

        _framebuffer->setRenderPass(clone(_pass));
        _framebuffer->setup();

        _tmpSemaphore = _framebuffer->device()->vkDevice().createSemaphore({});
    }

    /**/

    cmd::RenderCommandBuffer cmd {};
    cmd.begin();
    cmd.beginAccelPass({ _pass.get(), _framebuffer.get() });
    cmd.beginSubPass({});

    cmd.bindGraphicsPipeline(clone(_compiled.pipeline).into<GraphicsPipeline>());
    cmd.drawDispatch(1uL, 0uL, 3uL, 0uL);

    cmd.endSubPass();
    cmd.endAccelPass();
    cmd.end();

    /**/

    auto translator = make_uptr<driver::vk::VkRCmdTranslator>();
    auto batch = (*translator)(&cmd);

    if (not rSceneDepth->barriers.empty()) {

        batch->_tmpWaits.insert_range(
            batch->_tmpWaits.end(),
            reinterpret_cast<Vector<VkSemaphore>&>(rSceneDepth->barriers)
        );

        for (auto i = batch->_tmpWaitFlags.size(); i < batch->_tmpWaits.size(); ++i) {
            batch->_tmpWaitFlags.push_back(
                VkPipelineStageFlagBits::VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                VkPipelineStageFlagBits::VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT
            );
        }

        batch->_tmpSignals.push_back(_tmpSemaphore);

        rSceneDepth->barriers.clear();
        rSceneDepth->barriers.push_back(_tmpSemaphore.operator VkSemaphore());
    }

    batch->commitAndDispose();
    delete batch;
}

/**/

#include "__tmp_helper.hpp"

smr<AccelerationEffect> build_test_effect() {

    auto vertexStage = make_smr<Stage>(
        StageFlagBits::eVertex
        // Vector<StageInput> {},
        // Vector<StageOutput> {}
    );

    auto fragmentStage = make_smr<Stage>(
        StageFlagBits::eFragment
        // Vector<StageInput> {},
        // Vector<StageOutput> {
        //     StageOutput {
        //         TransferToken::from("depth"), TransferType::eForward, TransferDataType::eF32, DataOutputRate {}
        //     }
        // }
    );

    /**/

    const auto vertexShaderCode = read_shader_file("__test__predepth.vs");

    vertexStage->setIntermediate(make_smr<lang::Intermediate>());
    vertexStage->getIntermediate()->lang.dialect = lang::Dialect::eVulkanGlsl460;
    vertexStage->getIntermediate()->lang.text.emplace_back(_STD move(vertexShaderCode));

    const auto fragmentShaderCode = read_shader_file("__test__predepth.fs");

    fragmentStage->setIntermediate(make_smr<lang::Intermediate>());
    fragmentStage->getIntermediate()->lang.dialect = lang::Dialect::eVulkanGlsl460;
    fragmentStage->getIntermediate()->lang.text.emplace_back(_STD move(fragmentShaderCode));

    /**/

    Guid guid;
    GuidGenerate(guid);

    return make_smr<AccelerationEffect>(
        _STD move(guid),
        "test-predepth-effect",
        Vector<smr<Stage>> { _STD move(vertexStage), _STD move(fragmentStage) }
        // Vector<smr<const Symbol>> {},
        // Vector<smr<const Symbol>> { makeSceneDepthSymbol() }
    );
}

smr<const GraphicsPass> build_test_pass() {

    constexpr auto passFactory = VkAccelerationPassFactory();
    auto graphicsPass = passFactory.buildGraphicsPass(
        {
            makeSceneDepthSymbol()
        },
        {}
    ).value();

    return graphicsPass;
}

EffectCompileResult build_test_pipeline(
    mref<smr<const AccelerationEffect>> effect_,
    mref<smr<const GraphicsPass>> pass_
) {

    auto profile = make_smr<EffectProfile>(
        EffectProfile {
            ._name = "Test-PreDepth-Profile",
            ._definitions = {}
        }
    );

    /**/

    auto spec = make_smr<SimpleEffectSpecification>(
        Vector<smr<const lang::Symbol>> {}
    );
    spec->setPassSpec(
        make_uptr<GraphicsPassSpecification>(
            GraphicsPassSpecification {
                .depthCompareOp = DepthCompareOp::eLessEqual,
                .stencilCompareOp = StencilCompareOp::eNever,
                .stencilFailOp = StencilOp::eKeep,
                .stencilPassOp = StencilOp::eKeep,
                .stencilDepthFailOp = StencilOp::eKeep,
                .stencilCompareMask = 0ul,
                .stencilWriteMask = 0ul,
                .primitiveTopology = PrimitiveTopology::eTriangleList,
                .pass = pass_.get()
            }
        )
    );

    /**/

    const auto compiler = makeVkAccCompiler();
    auto result = compiler->compile(
        {
            .effect = _STD move(effect_),
            .profile = _STD move(profile),
            .spec = _STD move(spec)
        }
    );

    return result;
}
