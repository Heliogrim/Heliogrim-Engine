#include "TmpBrdfIrradPass.hpp"

#include <Engine.Accel.Command/CommandBuffer.hpp>
#include <Engine.Accel.Compile/VkEffectCompiler.hpp>
#include <Engine.Accel.Compile/Profile/EffectProfile.hpp>
#include <Engine.Accel.Compile/Spec/SimpleEffectSpecification.hpp>
#include <Engine.Accel.Pass/VkAccelerationPassFactory.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Driver.Vulkan/VkRCmdTranslator.hpp>
#include <Engine.GFX.RenderGraph/Relation/TextureDescription.hpp>
#include <Engine.GFX.RenderGraph/Symbol/ScopedSymbolContext.hpp>
#include <Engine.GFX/Texture/TextureFactory.hpp>
#include <Engine.GFX.Render.Command/RenderCommandBuffer.hpp>
#include <Engine.GFX/Graphics.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>
#include <Engine.Reflect/Cast.hpp>
#include <Engine.Accel.Pipeline/GraphicsPipeline.hpp>
#include <Engine.Accel.Pipeline/VkGraphicsPipeline.hpp>
#include <Engine.Common/Math/Coordinates.hpp>
#include <Engine.GFX.Loader/Texture/TextureResource.hpp>
#include <Engine.GFX.Render.Predefined/Symbols/BrdfIrradiance.hpp>
#include <Engine.GFX.Render.Predefined/Symbols/SceneView.hpp>
#include <Engine.GFX.Scene/View/SceneView.hpp>
#include <Engine.GFX/Buffer/BufferFactory.hpp>
#include <Engine.GFX/Buffer/UniformBufferView.hpp>
#include <Engine.GFX/Scene/SkyboxModel.hpp>
#include <Engine.GFX/Texture/SparseTextureView.hpp>
#include <Engine.Render.Scene/RenderSceneSystem.hpp>

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

void TmpBrdfIrradPass::destroy() noexcept {
    SubPass::destroy();

    auto device = Engine::getEngine()->getGraphics()->getCurrentDevice();
    device->vkDevice().destroySemaphore(static_cast<VkSemaphore>(_STD exchange(_tmpSignal, nullptr)));

    _resources.outBrdfIrradTexture->destroy<smr<TextureLikeObject>, graph::TextureDescription>();

    _brdfIrrad->destroy();
    _brdfIrrad.reset();
}

void TmpBrdfIrradPass::declareOutputs(ref<graph::ScopedSymbolContext> symCtx_) noexcept {
    symCtx_.registerExposeSymbol(makeBrdfIrradianceSymbol(), &_resources.outBrdfIrradTexture);
}

void TmpBrdfIrradPass::iterate(cref<graph::ScopedSymbolContext> symCtx_) noexcept {

    if (_effect.empty()) {
        _effect = build_test_effect();
    }

    if (_pass.empty()) {
        _pass = build_test_pass();
    }

    /**/

    ptr<const SkyboxModel> model = nullptr;
    auto sceneViewRes = symCtx_.getImportSymbol(makeSceneViewSymbol());
    auto sceneView = sceneViewRes->load<smr<const gfx::scene::SceneView>>();

    const auto sys = sceneView->getRenderSceneSystem();
    sys->getRegistry().forEach<SkyboxModel>(
        [&model](const auto& model_) {
            model = std::addressof(model_);
        }
    );

    /**/

    if (model == nullptr) {
        return;
    }

    /**/

    _skyboxMaterial = model->material(0uL);
}

void TmpBrdfIrradPass::resolve() noexcept {
    if (_effect && _compiled.flag == EffectCompileResultFlag::eUnknown) {
        _compiled = build_test_pipeline(clone(_effect), clone(_pass));
    }
}

void TmpBrdfIrradPass::execute(cref<graph::ScopedSymbolContext> symCtx_) noexcept {

    if (_tmpSignal == nullptr) {
        const auto device = Engine::getEngine()->getGraphics()->getCurrentDevice();
        _tmpSignal = device->vkDevice().createSemaphore({});
    }

    /**/

    if (_brdfIrrad.empty()) {

        const auto* const factory = TextureFactory::get();
        auto brdfIrrad = factory->build(
            {
                math::uivec3 { 64uL, 64uL, 1uL },
                TextureFormat::eR32G32B32A32Sfloat,
                MAX(static_cast<u32>(_STD floorf(_STD log2f(64.F))) + 1uL, 1uL),
                TextureType::eCube,
                vk::ImageAspectFlagBits::eColor,
                vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
                vk::MemoryPropertyFlagBits::eDeviceLocal,
                vk::SharingMode::eExclusive
            }
        );
        factory->buildView(
            brdfIrrad,
            {
                .layers = { 0L, 1L }, .type = TextureType::eCube,
                .mipLevels = { 0L, static_cast<s32>(brdfIrrad.mipLevels()) }
            }
        );

        _brdfIrrad = make_smr<Texture>(_STD move(brdfIrrad));
    }

    /**/

    if (_resources.outBrdfIrradTexture->empty()) {

        _resources.outBrdfIrradTexture.obj()->symbol = makeBrdfIrradianceSymbol();
        _resources.outBrdfIrradTexture.obj()->owned = true;

        _resources.outBrdfIrradTexture->create<
            smr<TextureLikeObject>,
            graph::TextureDescription,
            decltype([](const auto& obj_) {
                return obj_.get();
            })
        >(clone(_brdfIrrad).into<TextureLikeObject>());
    } else {
        _resources.outBrdfIrradTexture->store<
            smr<TextureLikeObject>,
            graph::TextureDescription,
            decltype([](const auto& obj_) {
                return obj_.get();
            })
        >(clone(_brdfIrrad).into<TextureLikeObject>());
    }

    /**/

    const_cast<ref<graph::ScopedSymbolContext>>(symCtx_).eraseExposedSymbol(makeBrdfIrradianceSymbol());
    const_cast<ref<graph::ScopedSymbolContext>>(symCtx_).exposeSymbol(
        makeBrdfIrradianceSymbol(),
        _resources.outBrdfIrradTexture.obj()
    );

    /**/

    assert(_effect);
    assert(_compiled.pipeline);

    if (_skyboxMaterial.empty() || _stored) {
        return;
    }
    _stored = true;

    /**/

    auto bufferFactory = BufferFactory::get();
    auto viewMatrixBuffer = bufferFactory->build(
        {
            sizeof(math::mat4) * 6uL,
            0,
            MemoryProperty::eHostVisible,
            vk::BufferCreateFlags {},
            vk::BufferUsageFlagBits::eUniformBuffer,
            vk::SharingMode::eExclusive
        }
    );

    Vector<math::mat4> viewMatrices = {
        math::mat4::make_identity().rotate(glm::radians(90.F), math::vec3_yaw)
                                   .rotate(glm::radians(180.F), math::vec3_pitch),
        math::mat4::make_identity().rotate(glm::radians(-90.F), math::vec3_yaw)
                                   .rotate(glm::radians(180.F), math::vec3_pitch),
        math::mat4::make_identity().rotate(glm::radians(-90.F), math::vec3_pitch),
        math::mat4::make_identity().rotate(glm::radians(90.F), math::vec3_pitch),
        math::mat4::make_identity().rotate(glm::radians(180.F), math::vec3_pitch),
        math::mat4::make_identity().rotate(glm::radians(180.F), math::vec3_roll)
    };

    for (auto& matrix : viewMatrices) {
        matrix = math::perspective(math::pih_f, 1.F, .1F, 512.F) * matrix;
    }

    viewMatrixBuffer.write<math::mat4>(viewMatrices.data(), 6uL);

    auto viewMatrixView = UniformBufferView();
    viewMatrixView.storeBuffer(&viewMatrixBuffer);

    /**/

    auto textureFactory = TextureFactory::get();
    auto tmpIrradImage = make_smr<Texture>(
        textureFactory->build(
            {
                _brdfIrrad->extent(),
                _brdfIrrad->format(),
                1uL,
                _brdfIrrad->type(),
                vk::ImageAspectFlagBits::eColor,
                vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc,
                vk::MemoryPropertyFlagBits::eDeviceLocal,
                vk::SharingMode::eExclusive
            }
        )
    );
    textureFactory->buildView(*tmpIrradImage);

    auto framebuffer = make_smr<Framebuffer>(Engine::getEngine()->getGraphics()->getCurrentDevice());

    framebuffer->addAttachment(clone(tmpIrradImage));
    framebuffer->setExtent(_brdfIrrad->extent());

    // Requires explicitly one layer due to multiview
    //framebuffer->setLayer(_brdfIrrad->layer());

    framebuffer->setRenderPass(clone(_pass));
    framebuffer->setup();

    /**/

    auto tmpSkySampler = TextureSampler();
    tmpSkySampler.setup(framebuffer->device());

    /**/

    cmd::RenderCommandBuffer cmd {};
    cmd.begin();

    cmd.lambda(
        [this](ref<AccelCommandBuffer> cmd_) {

            const auto barrier = vk::ImageMemoryBarrier {
                vk::AccessFlags(),
                vk::AccessFlags(),
                vk::ImageLayout::eUndefined,
                vk::ImageLayout::eTransferDstOptimal,
                VK_QUEUE_FAMILY_IGNORED,
                VK_QUEUE_FAMILY_IGNORED,
                _brdfIrrad->buffer().image(),
                vk::ImageSubresourceRange {
                    vk::ImageAspectFlagBits::eColor,
                    0uL,
                    _brdfIrrad->mipLevels(),
                    0uL,
                    6uL
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

            _brdfIrrad->buffer()._vkLayout = vk::ImageLayout::eTransferDstOptimal;
        }
    );

    struct IrradPushBlock {
        float deltaPhi = 2.0F * math::pi_f / 180.0F;
        float deltaTheta = 0.5F * math::pi_f / 64.0F;
    } pushBlock;

    for (auto mip = 0uL; mip < _brdfIrrad->mipLevels(); ++mip) {

        /**/

        cmd.beginAccelPass({ _pass.get(), framebuffer.get(), { vk::ClearColorValue { 0.F, 0.F, .2F, .1F } } });
        cmd.beginSubPass();

        cmd.bindGraphicsPipeline(clone(_compiled.pipeline).into<GraphicsPipeline>());

        /**/

        cmd.lambda(
            [this, mip, cpyPushBlock = pushBlock](ref<AccelCommandBuffer> cmd_) {

                vk::Viewport viewport {
                    0.F, 0.F, static_cast<float>(_brdfIrrad->extent().x >> mip),
                    static_cast<float>(_brdfIrrad->extent().y >> mip), 0.F, 1.F
                };
                cmd_.vkCommandBuffer().setViewport(0uL, 1uL, &viewport);

                /**/

                cmd_.vkCommandBuffer().pushConstants(
                    reinterpret_cast<VkPipelineLayout>(
                        Cast<VkGraphicsPipeline>(
                            _compiled.pipeline.get()
                        )->_vkPipeLayout
                    ),
                    vk::ShaderStageFlagBits::eFragment,
                    0uL,
                    sizeof(IrradPushBlock),
                    &cpyPushBlock
                );
            }
        );

        /**/

        const auto guard = _skyboxMaterial->acquire(resource::ResourceUsageFlag::eRead);
        const auto texture = guard->getParam<smr<TextureResource>>(material::ParameterIdentifier { 0 });
        const auto textureGuard = texture->acquire(resource::ResourceUsageFlag::eRead);

        if (auto tmpTex = Cast<SparseTextureView>(textureGuard->get())) {
            if (tmpTex->vkImageView() == nullptr) {
                textureFactory->buildView(*tmpTex);
            }
        }

        cmd.bindUniform(lang::SymbolId::from("view-matrix"sv), &viewMatrixView);
        cmd.bindTexture(lang::SymbolId::from("skybox"sv), textureGuard->get());
        cmd.bindTextureSampler(lang::SymbolId::from("skybox-sampler"sv), &tmpSkySampler);

        cmd.drawDispatch(1uL, 0uL, 36uL, 0uL);

        /**/

        cmd.endSubPass();
        cmd.endAccelPass();

        /**/

        cmd.lambda(
            [this, mip, tmpIrradImage](ref<AccelCommandBuffer> cmd_) {

                const auto beforeCopyBarrier = vk::ImageMemoryBarrier {
                    vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite,
                    vk::AccessFlagBits::eTransferRead,
                    vk::ImageLayout::eColorAttachmentOptimal,
                    vk::ImageLayout::eTransferSrcOptimal,
                    VK_QUEUE_FAMILY_IGNORED,
                    VK_QUEUE_FAMILY_IGNORED,
                    tmpIrradImage->buffer().image(),
                    vk::ImageSubresourceRange {
                        vk::ImageAspectFlagBits::eColor,
                        0uL,
                        1uL,
                        0uL,
                        6uL
                    }
                };

                cmd_.vkCommandBuffer().pipelineBarrier(
                    vk::PipelineStageFlagBits::eColorAttachmentOutput,
                    vk::PipelineStageFlagBits::eTransfer,
                    vk::DependencyFlagBits::eByRegion,
                    0uL,
                    nullptr,
                    0uL,
                    nullptr,
                    1uL,
                    &beforeCopyBarrier
                );

                tmpIrradImage->buffer()._vkLayout = vk::ImageLayout::eTransferSrcOptimal;

                /**/

                vk::ImageCopy copy {
                    vk::ImageSubresourceLayers { vk::ImageAspectFlagBits::eColor, 0uL, 0uL, 6uL },
                    vk::Offset3D {},
                    vk::ImageSubresourceLayers { vk::ImageAspectFlagBits::eColor, mip, 0uL, 6uL },
                    vk::Offset3D {},
                    vk::Extent3D { _brdfIrrad->extent().x >> mip, _brdfIrrad->extent().y >> mip, 1uL }
                };

                cmd_.vkCommandBuffer().copyImage(
                    tmpIrradImage->buffer().image(),
                    vk::ImageLayout::eTransferSrcOptimal,
                    _brdfIrrad->buffer().image(),
                    vk::ImageLayout::eTransferDstOptimal,
                    1uL,
                    &copy
                );

                /**/

                const auto afterCopyBarrier = vk::ImageMemoryBarrier {
                    vk::AccessFlagBits::eTransferRead,
                    vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite,
                    vk::ImageLayout::eTransferSrcOptimal,
                    vk::ImageLayout::eColorAttachmentOptimal,
                    VK_QUEUE_FAMILY_IGNORED,
                    VK_QUEUE_FAMILY_IGNORED,
                    tmpIrradImage->buffer().image(),
                    vk::ImageSubresourceRange {
                        vk::ImageAspectFlagBits::eColor,
                        0uL,
                        1uL,
                        0uL,
                        6uL
                    }
                };

                cmd_.vkCommandBuffer().pipelineBarrier(
                    vk::PipelineStageFlagBits::eTransfer,
                    vk::PipelineStageFlagBits::eColorAttachmentOutput,
                    vk::DependencyFlagBits::eByRegion,
                    0uL,
                    nullptr,
                    0uL,
                    nullptr,
                    1uL,
                    &afterCopyBarrier
                );
            }
        );
    }

    cmd.lambda(
        [this](ref<AccelCommandBuffer> cmd_) {

            const auto barrier = vk::ImageMemoryBarrier {
                vk::AccessFlagBits::eTransferWrite,
                vk::AccessFlagBits::eShaderRead,
                vk::ImageLayout::eTransferDstOptimal,
                vk::ImageLayout::eShaderReadOnlyOptimal,
                VK_QUEUE_FAMILY_IGNORED,
                VK_QUEUE_FAMILY_IGNORED,
                _brdfIrrad->buffer().image(),
                vk::ImageSubresourceRange {
                    vk::ImageAspectFlagBits::eColor,
                    0uL,
                    _brdfIrrad->mipLevels(),
                    0uL,
                    6uL
                }
            };

            cmd_.vkCommandBuffer().pipelineBarrier(
                vk::PipelineStageFlagBits::eTransfer,
                vk::PipelineStageFlagBits::eFragmentShader,
                vk::DependencyFlagBits::eByRegion,
                0uL,
                nullptr,
                0uL,
                nullptr,
                1uL,
                &barrier
            );

            _brdfIrrad->buffer()._vkLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
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
            reinterpret_cast<Vector<VkSemaphore>&>(_resources.outBrdfIrradTexture->barriers)
        );
        for (auto i = batch->_tmpWaitFlags.size(); i < batch->_tmpWaits.size(); ++i) {
            batch->_tmpWaitFlags.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
        }
        batch->_tmpSignals.push_back(reinterpret_cast<VkSemaphore>(_tmpSignal));

        _resources.outBrdfIrradTexture->barriers.clear();
        _resources.outBrdfIrradTexture->barriers.push_back(reinterpret_cast<VkSemaphore>(_tmpSignal));
    }

    batch->commitAndDispose();
    nativeBatch.reset();

    /**/

    tmpSkySampler.destroy();
    viewMatrixBuffer.destroy();

    framebuffer->destroy();
    framebuffer.reset();
    tmpIrradImage->destroy();
    tmpIrradImage.reset();

    /**/

    _brdfIrrad->buffer()._vkLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
}

/**/

#include "__tmp_helper.hpp"

smr<AccelerationEffect> build_test_effect() {

    using namespace ::hg::engine::accel::lang;

    auto vertexStage = make_smr<Stage>(StageFlagBits::eVertex);
    auto fragmentStage = make_smr<Stage>(StageFlagBits::eFragment);

    /**/

    const auto vertexShaderCode = read_shader_file("__test__brdfIrrad.gen.vs");

    vertexStage->setIntermediate(make_smr<Intermediate>());
    vertexStage->getIntermediate()->lang.dialect = Dialect::eVulkanGlsl460;
    vertexStage->getIntermediate()->lang.text.emplace_back(_STD move(vertexShaderCode));

    const auto fragmentShaderCode = read_shader_file("__test__brdfIrrad.gen.fs");

    fragmentStage->setIntermediate(make_smr<Intermediate>());
    fragmentStage->getIntermediate()->lang.dialect = Dialect::eVulkanGlsl460;
    fragmentStage->getIntermediate()->lang.text.emplace_back(_STD move(fragmentShaderCode));

    /**/

    {
        auto var = make_uptr<Variable>();
        auto sym = make_uptr<Symbol>();

        var->type = Type { .category = TypeCategory::eObject, .objectType = ObjectType::eUnknown };
        var->annotation = make_uptr<SimpleAnnotation<AnnotationType::eExternalLinkage>>();
        var->annotation = make_uptr<SimpleAnnotation<AnnotationType::eReadonly>>(_STD move(var->annotation));
        var->annotation = make_uptr<SimpleAnnotation<AnnotationType::eUniform>>(_STD move(var->annotation));
        var->annotation = make_uptr<SymbolIdAnnotation>("view-matrix", _STD move(var->annotation));
        var->annotation = make_uptr<VkBindLocationAnnotation>(0L, 0L, 0L, _STD move(var->annotation));

        sym->symbolId = SymbolId::from("view-matrix"sv);
        sym->var.type = SymbolType::eVariableSymbol;
        sym->var.data = var.get();

        vertexStage->getIntermediate()->rep.globalScope.inbound.emplace_back(_STD move(var));
        vertexStage->getIntermediate()->rep.symbolTable.insert(_STD move(sym));
    }

    {
        auto var = make_uptr<Variable>();
        auto sym = make_uptr<Symbol>();

        var->type = Type {
            .category = TypeCategory::eTexture, .textureType = lang::TextureType::eTextureCube
        };
        var->annotation = make_uptr<SimpleAnnotation<AnnotationType::eExternalLinkage>>();
        var->annotation = make_uptr<SimpleAnnotation<AnnotationType::eReadonly>>(_STD move(var->annotation));
        var->annotation = make_uptr<SimpleAnnotation<AnnotationType::eUniform>>(_STD move(var->annotation));
        var->annotation = make_uptr<SymbolIdAnnotation>("skybox", _STD move(var->annotation));
        var->annotation = make_uptr<VkBindLocationAnnotation>(1L, 0L, 0L, _STD move(var->annotation));

        sym->symbolId = SymbolId::from("skybox"sv);
        sym->var.type = SymbolType::eVariableSymbol;
        sym->var.data = var.get();

        fragmentStage->getIntermediate()->rep.globalScope.inbound.emplace_back(_STD move(var));
        fragmentStage->getIntermediate()->rep.symbolTable.insert(_STD move(sym));
    }

    {
        auto tmpVar = make_uptr<Variable>();
        tmpVar->type = Type { .category = TypeCategory::eObject, .objectType = ObjectType::eSampler };
        tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eExternalLinkage>>();
        tmpVar->annotation = make_uptr<SymbolIdAnnotation>("skybox-sampler", _STD move(tmpVar->annotation));

        auto tmpSym = make_uptr<Symbol>(
            SymbolId::from("skybox-sampler"sv),
            VariableSymbol { SymbolType::eVariableSymbol, tmpVar.get() }
        );

        fragmentStage->getIntermediate()->rep.globalScope.inbound.emplace_back(_STD move(tmpVar));
        fragmentStage->getIntermediate()->rep.symbolTable.insert(_STD move(tmpSym));
    }

    {
        auto var = make_uptr<Variable>();
        auto sym = make_uptr<Symbol>();

        var->type = Type {
            .category = TypeCategory::eObject, .objectType = ObjectType::eUnknown
        };
        var->annotation = make_uptr<SimpleAnnotation<AnnotationType::eExternalLinkage>>();
        var->annotation = make_uptr<SimpleAnnotation<AnnotationType::eConstant>>(_STD move(var->annotation));
        var->annotation = make_uptr<SimpleAnnotation<AnnotationType::eReadonly>>(_STD move(var->annotation));
        var->annotation = make_uptr<SymbolIdAnnotation>("block", _STD move(var->annotation));

        sym->symbolId = SymbolId::from("block"sv);
        sym->var.type = SymbolType::eVariableSymbol;
        sym->var.data = var.get();

        fragmentStage->getIntermediate()->rep.globalScope.inbound.emplace_back(_STD move(var));
        fragmentStage->getIntermediate()->rep.symbolTable.insert(_STD move(sym));
    }

    auto tmpVar = make_uptr<Variable>();
    auto tmpSym = make_uptr<Symbol>();

    tmpVar->type = Type {
        .category = TypeCategory::eTexture, .textureType = lang::TextureType::eTexture2d
    };
    tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eForwardLinkage>>();
    tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eWriteonly>>(
        _STD move(tmpVar->annotation)
    );
    tmpVar->annotation = make_uptr<SymbolIdAnnotation>(
        "color",
        _STD move(tmpVar->annotation)
    );

    tmpSym->symbolId = SymbolId::from("color"sv);
    tmpSym->var.type = SymbolType::eVariableSymbol;
    tmpSym->var.data = tmpVar.get();

    fragmentStage->getIntermediate()->rep.globalScope.outbound.emplace_back(_STD move(tmpVar));
    fragmentStage->getIntermediate()->rep.symbolTable.insert(_STD move(tmpSym));

    /**/

    Guid guid;
    GuidGenerate(guid);

    return make_smr<AccelerationEffect>(
        _STD move(guid),
        "test-brdf-irrad-effect",
        Vector<smr<Stage>> { _STD move(vertexStage), _STD move(fragmentStage) }
    );
}

smr<const GraphicsPass> build_test_pass() {

    constexpr auto passFactory = VkAccelerationPassFactory();
    auto graphicsPass = passFactory.buildGraphicsPass(
        { makeBrdfIrradianceSymbol() },
        {},
        {},
        0b0000'0000'0011'1111uL,
        0x0uL
    ).value();

    return graphicsPass;
}

EffectCompileResult build_test_pipeline(
    mref<smr<const AccelerationEffect>> effect_,
    mref<smr<const GraphicsPass>> pass_
) {

    auto profile = make_smr<EffectProfile>(
        EffectProfile {
            ._name = "Test-BrdfIrrad-Profile",
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
                .faceCulling = FaceCulling::eBack,
                .faceMode = FaceMode::eFill,
                .faceWinding = FaceWinding::eCcw,
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
