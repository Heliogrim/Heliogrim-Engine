#include "TriTestPass.hpp"

#include <Engine.Accel.Compile/VkEffectCompiler.hpp>
#include <Engine.Accel.Compile/Profile/EffectProfile.hpp>
#include <Engine.Accel.Compile/Spec/SimpleEffectSpecification.hpp>
#include <Engine.Accel.Pass/VkAccelerationPassFactory.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.Common/Make.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.GFX.Render.Command/RenderCommandBuffer.hpp>
#include <Engine.Driver.Vulkan/VkRCmdTranslator.hpp>
#include <Engine.GFX/Graphics.hpp>
#include <Engine.GFX.Render.Predefined/Symbols/SceneColor.hpp>
#include <Engine.GFX.Render.Predefined/Symbols/SceneDepth.hpp>
#include <Engine.GFX.RenderGraph/Relation/TextureDescription.hpp>
#include <Engine.GFX.RenderGraph/Symbol/SymbolContext.hpp>
#include <Engine.GFX.RenderGraph/Symbol/ScopedSymbolContext.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>
#include <Engine.Reflect/Cast.hpp>
#include <Engine.Accel.Pipeline/GraphicsPipeline.hpp>

#include "__tmp_helper.hpp"

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

void TriTestPass::destroy() noexcept {
    MeshSubPass::destroy();

    auto device = Engine::getEngine()->getGraphics()->getCurrentDevice();

    device->vkDevice().destroySemaphore(_STD exchange(_tmpSignal, nullptr));

    _framebuffer->destroy();
    _framebuffer.reset();

    _compiled.pipeline.reset();
    _pass.reset();
    _effect.reset();
}

void TriTestPass::declareTransforms(ref<graph::ScopedSymbolContext> symCtx_) noexcept {
    symCtx_.registerImportSymbol(makeSceneDepthSymbol(), &_resources.trDepthBuffer);
    symCtx_.registerExportSymbol(makeSceneDepthSymbol(), &_resources.trDepthBuffer);
}

void TriTestPass::declareOutputs(ref<graph::ScopedSymbolContext> symCtx_) noexcept {
    // Error: Will fail
    symCtx_.registerExportSymbol(makeSceneColorSymbol(), &_resources.sceneColor);
}

void TriTestPass::iterate(cref<graph::ScopedSymbolContext> symCtx_) noexcept {
    if (_effect.empty()) {
        _effect = build_test_effect();
    }
    if (_pass.empty()) {
        _pass = build_test_pass();
    }
}

void TriTestPass::resolve() noexcept {
    if (_effect && _compiled.flag == EffectCompileResultFlag::eUnknown) {
        _compiled = build_test_pipeline(clone(_effect), clone(_pass));
    }
}

void TriTestPass::execute(cref<graph::ScopedSymbolContext> symCtx_) noexcept {

    assert(_effect);
    assert(_compiled.pipeline);

    /**/

    auto sceneColorRes = symCtx_.getExportSymbol(makeSceneColorSymbol());
    const auto& sceneColorTex = sceneColorRes->load<smr<const TextureLikeObject>>();

    auto sceneDepthRes = symCtx_.getImportSymbol(makeSceneDepthSymbol());
    const auto& sceneDepthTex = sceneDepthRes->load<smr<const TextureLikeObject>>();

    if (
        !_framebuffer.empty() &&
        (_framebuffer->attachments().front() != sceneColorTex || _framebuffer->attachments().back() != sceneDepthTex)
    ) {
        _framebuffer->device()->vkDevice().destroySemaphore(_tmpSignal);
        _framebuffer->destroy();
        _framebuffer.reset();
    }

    if (_framebuffer.empty()) {

        const Vector<smr<const graph::Symbol>> outputSymbols { makeSceneColorSymbol(), makeSceneDepthSymbol() };
        _framebuffer = make_smr<Framebuffer>(Engine::getEngine()->getGraphics()->getCurrentDevice());

        for (const auto& output : outputSymbols) {

            auto data = symCtx_.getExportSymbol(clone(output));

            #ifdef _DEBUG
            const auto isValid = data->valid<
                smr<const TextureLikeObject>,
                graph::TextureDescription,
                decltype([](const auto& obj_) {
                    return obj_.get();
                })
            >();
            assert(isValid);
            #endif

            const auto& texture = data->load<smr<TextureLikeObject>>();
            _framebuffer->addAttachment(clone(texture));
        }

        {
            auto resource = symCtx_.getExportSymbol(makeSceneColorSymbol());
            const auto& texture = resource->load<smr<const TextureLikeObject>>();
            const auto sceneColorExtent = Cast<Texture>(texture.get())->extent();
            _framebuffer->setExtent(sceneColorExtent);
        }

        _framebuffer->setRenderPass(clone(_pass));
        _framebuffer->setup();

        _tmpSignal = _framebuffer->device()->vkDevice().createSemaphore({});
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
    auto nativeBatch = (*translator)(&cmd);
    auto* const batch = static_cast<ptr<driver::vk::VkNativeBatch>>(nativeBatch.get());

    {
        batch->_tmpWaits.insert_range(
            batch->_tmpWaits.end(),
            reinterpret_cast<Vector<VkSemaphore>&>(sceneColorRes->barriers)
        );
        for (auto i = batch->_tmpWaitFlags.size(); i < batch->_tmpWaits.size(); ++i) {
            batch->_tmpWaitFlags.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
        }
        batch->_tmpSignals.push_back(_tmpSignal);

        sceneColorRes->barriers.clear();
        sceneColorRes->barriers.push_back(_tmpSignal.operator VkSemaphore());

        batch->_tmpWaits.insert_range(
            batch->_tmpWaits.end(),
            reinterpret_cast<Vector<VkSemaphore>&>(sceneDepthRes->barriers)
        );
        for (auto i = batch->_tmpWaitFlags.size(); i < batch->_tmpWaits.size(); ++i) {
            batch->_tmpWaitFlags.push_back(
                VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT
            );
        }

        // Warning: Assume color output raises after late fragment test -> transient barrier assumption
        // Warning: Semaphores are single-producer single-consumer -> we need one semaphore per resource synchronization
        // batch->_tmpSignals.push_back(_tmpSignal);
        // 
        // sceneDepthRes->barriers.clear();
        // sceneDepthRes->barriers.push_back(_tmpSignal.operator VkSemaphore());
    }

    batch->commitAndDispose();
    nativeBatch.reset();
}

/**/

smr<AccelerationEffect> build_test_effect() {

    auto vertexStage = make_smr<Stage>(
        StageFlagBits::eVertex
        // Vector<StageInput> {},
        // Vector<StageOutput> {
        //     StageOutput {
        //         TransferToken {}, TransferType::eForward, TransferDataType::eU8Vec3,
        //         DataOutputRate {}
        //     }
        // }
    );

    auto fragmentStage = make_smr<Stage>(
        StageFlagBits::eFragment
        // Vector<StageInput> {
        //     StageInput {
        //         TransferToken {}, TransferType::eForward, TransferDataType::eU8Vec3,
        //         DataInputRate::ePerInvocation
        //     },
        //     StageInput {
        //         TransferToken::from("depth"), TransferType::eForward, TransferDataType::eF32,
        //         DataInputRate::ePerInvocation
        //     }
        // },
        // Vector<StageOutput> {
        //     StageOutput {
        //         TransferToken {}, TransferType::eForward, TransferDataType::eU8Vec4,
        //         DataOutputRate {}
        //     }
        //     /*
        //     StageOutput {
        //         TransferToken::from("out/depth"), TransferType::eForward, TransferDataType::eF32,
        //         DataOutputRate {}
        //     }
        //      */
        // }
    );

    /**/

    const auto vertexShaderCode = read_shader_file("__test__render.vs");

    vertexStage->setIntermediate(make_smr<lang::Intermediate>());
    vertexStage->getIntermediate()->lang.dialect = lang::Dialect::eVulkanGlsl460;
    vertexStage->getIntermediate()->lang.text.emplace_back(_STD move(vertexShaderCode));

    const auto fragmentShaderCode = read_shader_file("__test__render.fs");

    fragmentStage->setIntermediate(make_smr<lang::Intermediate>());
    fragmentStage->getIntermediate()->lang.dialect = lang::Dialect::eVulkanGlsl460;
    fragmentStage->getIntermediate()->lang.text.emplace_back(_STD move(fragmentShaderCode));

    uptr<lang::Variable> tmpVar {};
    uptr<lang::Symbol> tmpSym {};

    tmpVar = make_uptr<lang::Variable>();
    tmpVar->annotation = make_uptr<lang::SimpleAnnotation<lang::AnnotationType::eForwardLinkage>>();
    tmpSym = make_uptr<lang::Symbol>(
        lang::SymbolId::from("color"sv),
        lang::VariableSymbol { lang::SymbolType::eVariableSymbol, tmpVar.get() }
    );

    fragmentStage->getIntermediate()->rep.globalScope.outbound.emplace_back(_STD move(tmpVar));
    fragmentStage->getIntermediate()->rep.symbolTable.emplace(_STD move(tmpSym));

    /**/

    Guid guid;
    GuidGenerate(guid);

    return make_smr<AccelerationEffect>(
        _STD move(guid),
        "test-render-effect",
        Vector<smr<Stage>> { _STD move(vertexStage), _STD move(fragmentStage) }
        // Vector<smr<const Symbol>> { /*makeSceneDepthSymbol()*/ },
        // Vector<smr<const Symbol>> { makeSceneColorSymbol() }
    );
}

smr<const GraphicsPass> build_test_pass() {

    constexpr auto passFactory = VkAccelerationPassFactory();
    auto graphicsPass = passFactory.buildGraphicsPass(
        {
            makeSceneColorSymbol()
        },
        {
            makeSceneColorSymbol(), makeSceneDepthSymbol()
        }
    ).value();

    return graphicsPass;
}

EffectCompileResult build_test_pipeline(
    mref<smr<const AccelerationEffect>> effect_,
    mref<smr<const GraphicsPass>> pass_
) {

    auto profile = make_smr<EffectProfile>(
        EffectProfile {
            ._name = "Test-Tris-Profile",
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
