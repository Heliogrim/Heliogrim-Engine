#include "MatTestPass.hpp"

#include <Engine.Accel.Command/CommandBuffer.hpp>
#include <Engine.Accel.Compile/VkEffectCompiler.hpp>
#include <Engine.Accel.Compile/Profile/EffectProfile.hpp>
#include <Engine.Accel.Compile/Spec/SimpleEffectSpecification.hpp>
#include <Engine.Accel.Pass/VkAccelerationPassFactory.hpp>
#include <Engine.Common/Math/Coordinates.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.GFX/Graphics.hpp>
#include <Engine.GFX.Render.Predefined/Symbols/SceneColor.hpp>
#include <Engine.GFX.Render.Predefined/Symbols/SceneDepth.hpp>
#include <Engine.GFX.Render.Predefined/Symbols/SceneView.hpp>
#include <Engine.GFX.RenderGraph/Symbol/ScopedSymbolContext.hpp>
#include <Engine.GFX.Scene/View/SceneView.hpp>
#include <Engine.GFX/Buffer/Buffer.hpp>
#include <Engine.GFX.Render.Command/RenderCommandBuffer.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>
#include <Engine.Reflect/ExactType.hpp>
#include <Engine.Scene/IRenderScene.hpp>
#include <Engine.Scene/Graph/SceneGraph.hpp>
#include <Heliogrim/SceneComponent.hpp>
#include <Engine.Accel.Pipeline/GraphicsPipeline.hpp>
#include <Engine.Driver.Vulkan/VkRCmdTranslator.hpp>
#include <Engine.GFX.Loader/Texture/TextureResource.hpp>
#include <Engine.GFX.Render.Predefined/Symbols/BrdfIrradiance.hpp>
#include <Engine.GFX.Render.Predefined/Symbols/BrdfLut.hpp>
#include <Engine.GFX.Render.Predefined/Symbols/BrdfPrefilter.hpp>
#include <Engine.GFX/Cache/GlobalCacheCtrl.hpp>
#include <Engine.GFX/Pool/SceneResourcePool.hpp>
#include <Engine.GFX/Texture/TextureFactory.hpp>
#include <Engine.Reflect/Cast.hpp>
#include <Engine.Gfx/Texture/TextureView.hpp>
#include <Engine.GFX/Texture/VirtualTexturePage.hpp>
#include <Engine.Gfx/Texture/VirtualTextureView.hpp>
#include <Engine.GFX.Scene.Model/StaticGeometryModel.hpp>

using namespace hg::engine::render;
using namespace hg::engine::accel;
using namespace hg::engine::gfx;
using namespace hg;

[[nodiscard]] static smr<const GraphicsPass> build_test_pass();

[[nodiscard]] static EffectCompileResult build_test_pipeline(
    mref<smr<const AccelerationEffect>> effect_,
    mref<smr<const GraphicsPass>> pass_
);

/**/

void MatTestPass::destroy() noexcept {
    MeshSubPass::destroy();

    auto device = Engine::getEngine()->getGraphics()->getCurrentDevice();
    device->vkDevice().destroySemaphore(_STD exchange(_tmpSignal, nullptr));

    _cameraBufferView.reset();
    if (_cameraBuffer.memory->mapping) {
        _cameraBuffer.unmap();
    }
    _cameraBuffer.destroy();

    _staticInstanceView.reset();

    _sampler->destroy();
    _sampler.reset();

    _framebuffer->destroy();
    _framebuffer.reset();

    _compiled.pipeline.reset();
    _pass.reset();
    _materialEffect.effect.reset();
}

void MatTestPass::declareTransforms(ref<graph::ScopedSymbolContext> symCtx_) noexcept {
    MeshSubPass::declareTransforms(symCtx_);

    symCtx_.registerImportSymbol(makeSceneColorSymbol(), &_resources.inOutSceneColor);
    symCtx_.registerExportSymbol(makeSceneColorSymbol(), &_resources.inOutSceneColor);

    symCtx_.registerImportSymbol(makeSceneDepthSymbol(), &_resources.inOutDepthBuffer);
    symCtx_.registerExportSymbol(makeSceneDepthSymbol(), &_resources.inOutDepthBuffer);
}

void MatTestPass::iterate(cref<graph::ScopedSymbolContext> symCtx_) noexcept {

    if (_pass.empty()) {
        _pass = build_test_pass();
    }

    Vector<ptr<gfx::scene::StaticGeometryModel>> models {};
    models.reserve(_materials.size());

    auto sceneViewRes = symCtx_.getImportSymbol(makeSceneViewSymbol());
    auto sceneView = sceneViewRes->load<smr<const gfx::scene::SceneView>>();

    auto scene = sceneView->getScene()->renderGraph();
    scene->traversal(
        [&models](const ptr<scene::SceneGraph<gfx::scene::SceneModel>::node_type> node_) -> bool {

            auto size = node_->exclusiveSize();
            for (decltype(size) i = 0; i < size; ++i) {

                auto el = node_->elements()[i];
                if (ExactType<gfx::scene::StaticGeometryModel>(*el)) {
                    models.emplace_back(static_cast<gfx::scene::StaticGeometryModel*>(el));
                }

            }
            return true;
        }
    );

    /**/

    if (models.empty()) {
        _materials.clear();
        return;
    }

    /**/

    _materials.reserve(models.size());
    _instances.reserve(models.size());

    _materials.clear();
    _instances.clear();

    size_t idx = 0;
    for (auto* model : models) {

        auto material = model->material(0uL);
        auto guard = material->acquire(resource::ResourceUsageFlag::eRead);

        auto proto = guard->getPrototype();
        assert(proto->getMaterialEffects().size() == 1uLL);

        if (not _materialEffect.effect.empty()) {
            assert(proto->getMaterialEffects().front().effect == _materialEffect.effect);
        }

        if (_materialEffect.effect.empty()) {
            _materialEffect = clone(proto->getMaterialEffects().front());
        }

        _materials.emplace_back(material);
        _instances.emplace_back(model);

        /**/

        ++idx;
    }
}

void MatTestPass::resolve() noexcept {
    if (_materialEffect.effect && _compiled.flag == EffectCompileResultFlag::eUnknown) {
        _compiled = build_test_pipeline(clone(_materialEffect.effect), clone(_pass));
    }
}

void MatTestPass::execute(cref<graph::ScopedSymbolContext> symCtx_) noexcept {

    if (!_materialEffect.effect || !_compiled.pipeline) {
        return;
    }

    /**/

    auto sceneColorRes = symCtx_.getExportSymbol(makeSceneColorSymbol());
    auto sceneDepthRes = symCtx_.getImportSymbol(makeSceneDepthSymbol());

    const auto& sceneColorTex = sceneColorRes->load<smr<const TextureLikeObject>>();
    const auto& sceneDepthTex = sceneDepthRes->load<smr<const TextureLikeObject>>();

    /**/

    if (
        not _framebuffer.empty() &&
        (_framebuffer->attachments().front() != sceneColorTex || _framebuffer->attachments().back() != sceneDepthRes)
    ) {
        _framebuffer->device()->vkDevice().destroySemaphore(_tmpSignal);
        _framebuffer->destroy();
        _framebuffer.reset();
    }

    if (_framebuffer.empty()) {

        const Vector outputSymbols { makeSceneColorSymbol(), makeSceneDepthSymbol() };
        _framebuffer = make_smr<Framebuffer>(Engine::getEngine()->getGraphics()->getCurrentDevice());

        for (const auto& output : outputSymbols) {

            auto data = symCtx_.getExportSymbol(clone(output));

            const auto& texture = data->load<smr<TextureLikeObject>>();
            _framebuffer->addAttachment(clone(texture));
        }

        {
            const auto texture = Cast<Texture>(sceneColorTex.get());
            _framebuffer->setExtent(texture->extent());
        }

        _framebuffer->setRenderPass(clone(_pass));
        _framebuffer->setup();

        _tmpSignal = _framebuffer->device()->vkDevice().createSemaphore({});
    }

    /**/

    if (_sampler == nullptr) {
        _sampler = make_uptr<TextureSampler>();
        _sampler->setup(_framebuffer->device());
    }

    /**/

    if (_cameraBuffer.buffer == nullptr) {

        const auto& device = _framebuffer->device();

        vk::BufferCreateInfo bci {
            vk::BufferCreateFlags {}, sizeof(math::mat4) * 3, vk::BufferUsageFlagBits::eUniformBuffer,
            vk::SharingMode::eExclusive, 0uL, nullptr
        };
        _cameraBuffer.buffer = device->vkDevice().createBuffer(bci);
        _cameraBuffer.device = device->vkDevice();

        const auto allocResult = memory::allocate(
            device->allocator(),
            device,
            _cameraBuffer.buffer,
            MemoryProperties { MemoryProperty::eHostVisible },
            _cameraBuffer.memory
        );

        assert(_cameraBuffer.buffer);
        assert(_cameraBuffer.memory);

        _cameraBuffer.size = _cameraBuffer.memory->size;
        _cameraBuffer.usageFlags = vk::BufferUsageFlagBits::eUniformBuffer;

        _cameraBuffer.bind();
    }

    auto sceneViewRes = symCtx_.getImportSymbol(makeSceneViewSymbol());
    auto sceneView = sceneViewRes->load<smr<gfx::scene::SceneView>>();

    struct TmpStruct {
        math::mat4 mvp;
        math::mat4 proj;
        math::mat4 view;
    };

    auto tmp = TmpStruct {
        sceneView->getProjectionMatrix() * sceneView->getViewMatrix(),
        sceneView->getProjectionMatrix(),
        sceneView->getViewMatrix()
    };

    _cameraBuffer.write<TmpStruct>(&tmp, 1uL);

    if (_cameraBufferView == nullptr) {
        _cameraBufferView = make_uptr<UniformBufferView>();
        _cameraBufferView->storeBuffer(&_cameraBuffer);
    }

    /**/

    if (_staticInstanceView == nullptr) {
        _staticInstanceView = make_uptr<gfx::StorageBufferView>();
    }

    const auto& staticInstancePool = sceneView->getScene()->getSceneResourcePool()->staticInstancePool;
    _staticInstanceView->storeBuffer(staticInstancePool.getPoolView());

    /**/

    cmd::RenderCommandBuffer cmd {};
    cmd.begin();
    cmd.beginAccelPass({ _pass.get(), _framebuffer.get() });
    cmd.beginSubPass({});

    cmd.bindGraphicsPipeline(clone(_compiled.pipeline).into<GraphicsPipeline>());

    const auto* const gpipe = static_cast<const ptr<const GraphicsPipeline>>(_compiled.pipeline.get());
    const auto& alias = _compiled.alias;

    for (size_t idx = 0; idx < _instances.size(); ++idx) {

        auto& model = _instances[idx];
        auto& modelResource = model->geometryResource();
        auto& materialResource = _materials[idx];

        auto modelGuard = modelResource->acquire(resource::ResourceUsageFlag::eRead);
        auto materialGuard = materialResource->acquire(resource::ResourceUsageFlag::eRead);

        const auto& indexBuffer = modelGuard->indices();
        const auto& vertexBuffer = modelGuard->vertices();

        cmd.lambda(
            [&indexBuffer, &vertexBuffer](ref<AccelCommandBuffer> cmd_) {
                cmd_.bindIndexBuffer(indexBuffer.get());
                cmd_.bindVertexBuffer(0uL, vertexBuffer.get());
            }
        );

        for (const auto& group : gpipe->getBindingLayout().groups) {
            for (const auto& element : group.elements) {

                const auto symbolId = element.symbol->symbolId;
                const auto aliasId = alias.aliasOrValue(symbolId);

                const auto iter = _STD ranges::find(
                    _materialEffect.pattern->st.pairs,
                    aliasId,
                    [](const auto& pair_) {
                        return pair_.second;
                    }
                );

                if (iter != _materialEffect.pattern->st.pairs.end()) {

                    const auto texture = materialGuard->getParam<smr<TextureResource>>(iter->first);
                    auto textureGuard = texture->acquire(resource::ResourceUsageFlag::eRead);

                    if (auto textureView = Cast<TextureView>(textureGuard.imm()->get())) {
                        cmd.bindTexture(clone(symbolId), textureView, _sampler.get());

                    } else if (auto virtualTextureView = Cast<VirtualTextureView>(textureGuard.imm()->get())) {

                        if (virtualTextureView->vkImageView() == nullptr) {
                            TextureFactory::get()->buildView(*virtualTextureView, {});
                        }

                        /**/
                        if (virtualTextureView->pages().front()->memory()->state() != VirtualMemoryPageState::eLoaded) {

                            textureGuard.release();

                            auto* cc = Engine::getEngine()->getGraphics()->cacheCtrl();
                            for (
                                u32 mipLevel = virtualTextureView->minMipLevel();
                                mipLevel < virtualTextureView->maxMipLevel();
                                ++mipLevel
                            ) {
                                cc->markLoadedAsUsed(
                                    clone(texture),
                                    cache::TextureSubResource {
                                        .layer = 0,
                                        .mip = mipLevel,
                                        .offset = math::uivec3 {},
                                        .extent = math::uivec3 {
                                            virtualTextureView->width(), virtualTextureView->height(),
                                            virtualTextureView->depth()
                                        }
                                    }
                                );
                            }
                        }
                        /**/

                        cmd.bindTexture(clone(symbolId), virtualTextureView, _sampler.get());

                    } else {
                        assert(false);
                    }

                    continue;
                }

                static auto viewSym = lang::SymbolId::from("view");
                if (aliasId == viewSym) {
                    cmd.bindUniform(clone(symbolId), _cameraBufferView.get());
                    continue;
                }

                static auto modelSym = lang::SymbolId::from("model");
                if (aliasId == modelSym) {
                    cmd.bindStorage(clone(symbolId), _staticInstanceView.get());
                    continue;
                }

                // TODO: Take care of brdf helper objects for test case: `brdf-lut`, `brdf-prefilter`, `brdf-irradiance`
                static auto brdfLutSym = lang::SymbolId::from("brdf-lut");
                if (aliasId == brdfLutSym) {

                    const auto res = symCtx_.getImportSymbol(makeBrdfLutSymbol());
                    const auto lutRes = res->load<smr<TextureLikeObject>>();
                    auto* const lutTex = Cast<Texture>(lutRes.get());

                    if (_brdfLutView == nullptr || _brdfLutView->owner() != lutTex) {
                        _brdfLutView = lutTex->makeView(
                            { 0uL, lutTex->layer() },
                            { lutTex->width(), lutTex->height(), lutTex->depth() },
                            { 0uL, lutTex->mipLevels() }
                        );
                    }

                    cmd.bindTexture(clone(symbolId), _brdfLutView.get(), _sampler.get());
                    continue;
                }

                static auto brdfPrefSym = lang::SymbolId::from("brdf-prefilter");
                if (aliasId == brdfPrefSym) {

                    const auto res = symCtx_.getImportSymbol(makeBrdfPrefilterSymbol());
                    const auto lutRes = res->load<smr<TextureLikeObject>>();
                    auto* const lutTex = Cast<Texture>(lutRes.get());

                    if (_brdfPrefView == nullptr || _brdfPrefView->owner() != lutTex) {
                        _brdfPrefView = lutTex->makeView(
                            { 0uL, lutTex->layer() },
                            { lutTex->width(), lutTex->height(), lutTex->depth() },
                            { 0uL, lutTex->mipLevels() }
                        );
                    }

                    cmd.bindTexture(clone(symbolId), _brdfPrefView.get(), _sampler.get());
                    continue;
                }

                static auto brdfIrradSym = lang::SymbolId::from("brdf-irradiance");
                if (aliasId == brdfIrradSym) {

                    const auto res = symCtx_.getImportSymbol(makeBrdfIrradianceSymbol());
                    const auto lutRes = res->load<smr<TextureLikeObject>>();
                    auto* const lutTex = Cast<Texture>(lutRes.get());

                    if (_brdfIrradView == nullptr || _brdfIrradView->owner() != lutTex) {
                        _brdfIrradView = lutTex->makeView(
                            { 0uL, lutTex->layer() },
                            { lutTex->width(), lutTex->height(), lutTex->depth() },
                            { 0uL, lutTex->mipLevels() }
                        );
                    }

                    cmd.bindTexture(clone(symbolId), _brdfIrradView.get(), _sampler.get());
                    continue;
                }

                __debugbreak();
            }
        }

        cmd.drawStaticMeshIdx(1uL, model->_sceneInstanceIndex, indexBuffer->size() / sizeof(u32) / 3uLL, 0uL);
    }

    cmd.endSubPass();
    cmd.endAccelPass();
    cmd.end();

    /**/

    auto translator = make_uptr<driver::vk::VkRCmdTranslator>();
    auto batch = (*translator)(&cmd);

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
    }

    batch->commitAndDispose();
    delete batch;
}

/**/

smr<const GraphicsPass> build_test_pass() {

    constexpr auto passFactory = VkAccelerationPassFactory {};
    auto graphicsPass = passFactory.buildGraphicsPass(
        { makeSceneColorSymbol(), /**/makeSceneDepthSymbol() },
        { makeSceneColorSymbol(), makeSceneDepthSymbol() }
    ).value();

    return graphicsPass;
}

EffectCompileResult build_test_pipeline(
    mref<smr<const AccelerationEffect>> effect_,
    mref<smr<const GraphicsPass>> pass_
) {

    auto profile = make_smr<EffectProfile>(
        EffectProfile {
            ._name = "Test-Mat-Profile",
            ._definitions = {}
        }
    );

    /**/

    auto spec = make_smr<SimpleEffectSpecification>();
    spec->setPassSpec(
        make_uptr<GraphicsPassSpecification>(
            GraphicsPassSpecification {
                .depthCompareOp = DepthCompareOp::eLessEqual,
                .stencilCompareOp = StencilCompareOp::eNever,
                .stencilFailOp = StencilOp::eKeep,
                .stencilPassOp = StencilOp::eKeep,
                .stencilDepthFailOp = StencilOp::eKeep,
                .stencilCompareMask = 0uL,
                .stencilWriteMask = 0uL,
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
