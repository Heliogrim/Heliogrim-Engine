#include "TmpBrdfLutPass.hpp"

#include <Engine.Accel.Command/CommandBuffer.hpp>
#include <Engine.Accel.Compile/VkEffectCompiler.hpp>
#include <Engine.Accel.Compile/Profile/EffectProfile.hpp>
#include <Engine.Accel.Compile/Spec/SimpleEffectSpecification.hpp>
#include <Engine.Accel.Pass/VkAccelerationPassFactory.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Driver.Vulkan/VkRCmdTranslator.hpp>
#include <Engine.GFX.Render.Predefined/Symbols/SceneColor.hpp>
#include <Engine.GFX.RenderGraph/Relation/TextureDescription.hpp>
#include <Engine.GFX.RenderGraph/Symbol/ScopedSymbolContext.hpp>
#include <Engine.GFX/Texture/TextureFactory.hpp>
#include <Engine.GFX.Render.Command/RenderCommandBuffer.hpp>
#include <Engine.GFX/Graphics.hpp>
#include <Engine.GFX.Render.Predefined/Symbols/BrdfLut.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>
#include <Engine.Reflect/Cast.hpp>
#include <Engine.Accel.Pipeline/GraphicsPipeline.hpp>

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

void TmpBrdfLutPass::destroy() noexcept {
    SubPass::destroy();

    auto device = Engine::getEngine()->getGraphics()->getCurrentDevice();
    device->vkDevice().destroySemaphore(static_cast<VkSemaphore>(std::exchange(_tmpSignal, nullptr)));

    _resources.outBrdfLutTexture->destroy<smr<TextureLikeObject>, graph::TextureDescription>();

    _brdfLut->destroy();
    _brdfLut.reset();
}

void TmpBrdfLutPass::declareOutputs(ref<graph::ScopedSymbolContext> symCtx_) noexcept {
    symCtx_.registerExposeSymbol(makeBrdfLutSymbol(), &_resources.outBrdfLutTexture);
}

void TmpBrdfLutPass::iterate(cref<graph::ScopedSymbolContext> symCtx_) noexcept {

    if (_effect.empty()) {
        _effect = build_test_effect();
    }

    if (_pass.empty()) {
        _pass = build_test_pass();
    }

}

void TmpBrdfLutPass::resolve() noexcept {
    if (_effect && _compiled.flag == EffectCompileResultFlag::eUnknown) {
        _compiled = build_test_pipeline(clone(_effect), clone(_pass));
    }
}

void TmpBrdfLutPass::execute(cref<graph::ScopedSymbolContext> symCtx_) noexcept {

    if (_tmpSignal == nullptr) {
        const auto device = Engine::getEngine()->getGraphics()->getCurrentDevice();
        _tmpSignal = device->vkDevice().createSemaphore({});
    }

    /**/

    if (_brdfLut.empty()) {
        const auto* const factory = TextureFactory::get();
        auto brdfLut = factory->build(
            {
                math::uivec3 { 512uL, 512uL, 1uL }, TextureFormat::eR16G16Sfloat, 1uL, TextureType::e2d,
                vk::ImageAspectFlagBits::eColor,
                vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
                vk::MemoryPropertyFlagBits::eDeviceLocal,
                vk::SharingMode::eExclusive
            }
        );
        factory->buildView(brdfLut, { .layers = { 0L, 1L }, .type = TextureType::e2d, .mipLevels = { 0L, 1L } });

        _brdfLut = make_smr<Texture>(std::move(brdfLut));
    }

    /**/

    if (_resources.outBrdfLutTexture->empty()) {

        _resources.outBrdfLutTexture.obj()->symbol = makeBrdfLutSymbol();
        _resources.outBrdfLutTexture.obj()->owned = true;

        _resources.outBrdfLutTexture->create<
            smr<TextureLikeObject>,
            graph::TextureDescription,
            decltype([](const auto& obj_) {
                return obj_.get();
            })
        >(clone(_brdfLut).into<TextureLikeObject>());
    } else {
        _resources.outBrdfLutTexture->store<
            smr<TextureLikeObject>,
            graph::TextureDescription,
            decltype([](const auto& obj_) {
                return obj_.get();
            })
        >(clone(_brdfLut).into<TextureLikeObject>());
    }

    /**/
    const_cast<ref<graph::ScopedSymbolContext>>(symCtx_).eraseExposedSymbol(makeBrdfLutSymbol());
    const_cast<ref<graph::ScopedSymbolContext>>(symCtx_).exposeSymbol(
        makeBrdfLutSymbol(),
        _resources.outBrdfLutTexture.obj()
    );

    /**/

    assert(_effect);
    assert(_compiled.pipeline);

    if (not _framebuffer.empty()) {
        return;
    }

    /**/

    if (_framebuffer.empty()) {

        _framebuffer = make_smr<Framebuffer>(Engine::getEngine()->getGraphics()->getCurrentDevice());

        _framebuffer->addAttachment(clone(_brdfLut));
        _framebuffer->setExtent(_brdfLut->extent());

        _framebuffer->setRenderPass(clone(_pass));
        _framebuffer->setup();
    }

    /**/

    cmd::RenderCommandBuffer cmd {};
    cmd.begin();

    cmd.lambda(
        [this](ref<AccelCommandBuffer> cmd_) {

            const auto barrier = vk::ImageMemoryBarrier {
                vk::AccessFlags(),
                vk::AccessFlags(),
                vk::ImageLayout::eUndefined,
                vk::ImageLayout::eColorAttachmentOptimal,
                VK_QUEUE_FAMILY_IGNORED,
                VK_QUEUE_FAMILY_IGNORED,
                _brdfLut->buffer().image(),
                vk::ImageSubresourceRange {
                    vk::ImageAspectFlagBits::eColor,
                    0uL,
                    _brdfLut->mipLevels(),
                    0uL,
                    1uL
                }
            };

            cmd_.vkCommandBuffer().pipelineBarrier(
                vk::PipelineStageFlagBits::eTopOfPipe,
                vk::PipelineStageFlagBits::eTopOfPipe,
                vk::DependencyFlags(),
                0uL,
                nullptr,
                0uL,
                nullptr,
                1uL,
                &barrier
            );

            _brdfLut->buffer()._vkLayout = vk::ImageLayout::eColorAttachmentOptimal;
        }
    );

    cmd.beginAccelPass({ _pass.get(), _framebuffer.get() });
    cmd.beginSubPass();

    cmd.bindGraphicsPipeline(clone(_compiled.pipeline).into<accel::GraphicsPipeline>());
    cmd.drawDispatch(1uL, 0uL, 6uL, 0uL);

    cmd.endSubPass();
    cmd.endAccelPass();

    cmd.lambda(
        [this](ref<AccelCommandBuffer> cmd_) {

            const auto barrier = vk::ImageMemoryBarrier {
                vk::AccessFlags(),
                vk::AccessFlags(),
                vk::ImageLayout::eColorAttachmentOptimal,
                vk::ImageLayout::eShaderReadOnlyOptimal,
                VK_QUEUE_FAMILY_IGNORED,
                VK_QUEUE_FAMILY_IGNORED,
                _brdfLut->buffer().image(),
                vk::ImageSubresourceRange {
                    vk::ImageAspectFlagBits::eColor,
                    0uL,
                    _brdfLut->mipLevels(),
                    0uL,
                    1uL
                }
            };

            cmd_.vkCommandBuffer().pipelineBarrier(
                vk::PipelineStageFlagBits::eTopOfPipe,
                vk::PipelineStageFlagBits::eTopOfPipe,
                vk::DependencyFlags(),
                0uL,
                nullptr,
                0uL,
                nullptr,
                1uL,
                &barrier
            );

        }
    );

    cmd.end();

    /**/

    auto translator = make_uptr<driver::vk::VkRCmdTranslator>();
    auto nativeBatch = (*translator)(&cmd);
    const auto batch = static_cast<ptr<driver::vk::VkNativeBatch>>(nativeBatch.get());

    {
        batch->_tmpWaits.insert_range(
            batch->_tmpWaits.end(),
            reinterpret_cast<Vector<VkSemaphore>&>(_resources.outBrdfLutTexture->barriers)
        );
        for (auto i = batch->_tmpWaitFlags.size(); i < batch->_tmpWaits.size(); ++i) {
            batch->_tmpWaitFlags.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
        }
        batch->_tmpSignals.push_back(reinterpret_cast<VkSemaphore>(_tmpSignal));

        _resources.outBrdfLutTexture->barriers.clear();
        _resources.outBrdfLutTexture->barriers.push_back(reinterpret_cast<VkSemaphore>(_tmpSignal));
    }

    batch->commitAndDispose();
    nativeBatch.reset();

    /**/

    _brdfLut->buffer()._vkLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
}

/**/

#include "__tmp_helper.hpp"

smr<AccelerationEffect> build_test_effect() {

    auto vertexStage = make_smr<Stage>(StageFlagBits::eVertex);
    auto fragmentStage = make_smr<Stage>(StageFlagBits::eFragment);

    /**/

    const auto vertexShaderCode = read_shader_file("__test__brdfLut.gen.vs");

    vertexStage->setIntermediate(make_smr<lang::Intermediate>());
    vertexStage->getIntermediate()->lang.dialect = lang::Dialect::eVulkanGlsl460;
    vertexStage->getIntermediate()->lang.text.emplace_back(std::move(vertexShaderCode));

    const auto fragmentShaderCode = read_shader_file("__test__brdfLut.gen.fs");

    fragmentStage->setIntermediate(make_smr<lang::Intermediate>());
    fragmentStage->getIntermediate()->lang.dialect = lang::Dialect::eVulkanGlsl460;
    fragmentStage->getIntermediate()->lang.text.emplace_back(std::move(fragmentShaderCode));

    /**/

    auto tmpVar = make_uptr<lang::Variable>();
    auto tmpSym = make_uptr<lang::Symbol>();

    tmpVar->type = lang::Type {
        .category = lang::TypeCategory::eTexture, .textureType = lang::TextureType::eTexture2d
    };
    tmpVar->annotation = make_uptr<lang::SimpleAnnotation<lang::AnnotationType::eForwardLinkage>>();
    tmpVar->annotation = make_uptr<lang::SimpleAnnotation<lang::AnnotationType::eWriteonly>>(
        std::move(tmpVar->annotation)
    );
    tmpVar->annotation = make_uptr<lang::SymbolIdAnnotation>(
        "color",
        std::move(tmpVar->annotation)
    );

    tmpSym->symbolId = lang::SymbolId::from("color"sv);
    tmpSym->var.type = lang::SymbolType::eVariableSymbol;
    tmpSym->var.data = tmpVar.get();

    fragmentStage->getIntermediate()->rep.globalScope.outbound.emplace_back(std::move(tmpVar));
    fragmentStage->getIntermediate()->rep.symbolTable.insert(std::move(tmpSym));

    /**/

    Guid guid;
    GuidGenerate(guid);

    return make_smr<AccelerationEffect>(
        std::move(guid),
        "test-brdf-lut-effect",
        Vector<smr<Stage>> { std::move(vertexStage), std::move(fragmentStage) }
    );
}

smr<const GraphicsPass> build_test_pass() {

    constexpr auto passFactory = VkAccelerationPassFactory();
    auto graphicsPass = passFactory.buildGraphicsPass(
        { makeBrdfLutSymbol() },
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
            ._name = "Test-BrdfLut-Profile",
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
                .depthCompareOp = DepthCompareOp::eNever,
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
            .effect = std::move(effect_),
            .profile = std::move(profile),
            .spec = std::move(spec)
        }
    );

    return result;
}
