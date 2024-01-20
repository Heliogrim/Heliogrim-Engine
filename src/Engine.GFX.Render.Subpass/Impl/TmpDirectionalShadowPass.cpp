#include "TmpDirectionalShadowPass.hpp"

#include <Engine.Common/GLM.hpp>
#include <Engine.Accel.Command/CommandBuffer.hpp>
#include <Engine.Accel.Compile/VkEffectCompiler.hpp>
#include <Engine.Accel.Compile/Profile/EffectProfile.hpp>
#include <Engine.Accel.Compile/Spec/SimpleEffectSpecification.hpp>
#include <Engine.Accel.Pass/VkAccelerationPassFactory.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.GFX/Graphics.hpp>
#include <Engine.GFX.Render.Predefined/Symbols/DirectionalShadow.hpp>
#include <Engine.GFX.Render.Predefined/Symbols/SceneView.hpp>
#include <Engine.GFX.RenderGraph/Symbol/ScopedSymbolContext.hpp>
#include <Engine.GFX.Scene/View/SceneView.hpp>
#include <Engine.GFX/Texture/TextureFactory.hpp>
#include <Engine.GFX.Render.Command/RenderCommandBuffer.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>
#include <Engine.Reflect/ExactType.hpp>
#include <Engine.Accel.Pipeline/GraphicsPipeline.hpp>
#include <Engine.Accel.Pipeline/VkGraphicsPipeline.hpp>
#include <Engine.Common/Math/Convertion.hpp>
#include <Engine.Common/Math/Coordinates.hpp>
#include <Engine.Driver.Vulkan/VkRCmdTranslator.hpp>
#include <Engine.GFX.Render.Predefined/Symbols/SceneDepth.hpp>
#include <Engine.GFX.RenderGraph/Relation/TextureDescription.hpp>
#include <Engine.GFX/Buffer/BufferFactory.hpp>
#include <Engine.GFX/Pool/SceneResourcePool.hpp>
#include <Engine.GFX/Texture/SparseTextureView.hpp>
#include <Engine.GFX/Texture/TextureView.hpp>
#include <Engine.Reflect/Cast.hpp>
#include <Engine.Render.Scene/RenderSceneSystem.hpp>

using namespace hg::engine::render;
using namespace hg::engine::accel;
using namespace hg::engine::gfx;
using namespace hg;

/**/

[[nodiscard]] static smr<material::MaterialEffect> build_test_effect();

[[nodiscard]] static smr<const GraphicsPass> build_test_pass();

[[nodiscard]] static EffectCompileResult build_test_pipeline(
    mref<smr<const AccelerationEffect>> effect_,
    mref<smr<const GraphicsPass>> pass_
);

/**/

TmpDirectionalShadowPass::TmpDirectionalShadowPass() = default;

TmpDirectionalShadowPass::~TmpDirectionalShadowPass() = default;

void TmpDirectionalShadowPass::setup(ref<graph::ScopedSymbolContext> symCtx_) noexcept {

    auto device = Engine::getEngine()->getGraphics()->getCurrentDevice();
    const auto* const textureFactory = TextureFactory::get();

    /**/

    assert(_dirShadowMap.empty());
    _dirShadowMap = make_smr<Texture>(
        textureFactory->build(
            {
                .extent = math::uivec3 { 2048uL, 2048uL, /*1uL*/cascadeCount },
                .format = TextureFormat::eD32Sfloat,
                .mipLevels = 1uL,
                .type = TextureType::e2dArray,
                .vkAspect = vk::ImageAspectFlagBits::eDepth,
                .vkUsage = vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled,
                .vkMemoryFlags = vk::MemoryPropertyFlagBits::eDeviceLocal,
                .vkSharing = vk::SharingMode::eExclusive
            }
        )
    );
    _dirShadowMap->buffer()._vkLayout = vk::ImageLayout::eDepthReadOnlyOptimal;
    textureFactory->buildView(*_dirShadowMap, {});

    for (auto cascade = 0u; cascade < cascadeCount; ++cascade) {

        _dirShadowCascades[cascade] = make_smr<TextureView>(
            _dirShadowMap->makeView(
                math::uivec2 { cascade, cascade },
                math::uExtent3D {
                    _dirShadowMap->width(), _dirShadowMap->height(), _dirShadowMap->depth(), 0uL, 0uL, 0uL
                },
                math::uivec2 { 0uL, 1uL }
            ).release()
        );
        textureFactory->buildView(
            *_dirShadowCascades[cascade],
            { math::uivec2 { cascade, cascade }, TextureType::e2dArray, math::ivec2 { 0uL, 1uL } }
        );
    }

    /**/

    assert(_resources.outDirShadows->empty());
    if (_resources.outDirShadows->empty()) {

        _resources.outDirShadows.obj()->symbol = makeDirectionalShadowSymbol();
        _resources.outDirShadows.obj()->owned = true;

        _resources.outDirShadows->create<
            smr<TextureLikeObject>,
            graph::TextureDescription,
            decltype([](const auto& obj_) {
                return obj_.get();
            })
        >(clone(_dirShadowMap).into<TextureLikeObject>());
    }

    /**/

    MeshSubPass::setup(symCtx_);
}

void TmpDirectionalShadowPass::destroy() noexcept {
    MeshSubPass::destroy();

    auto device = Engine::getEngine()->getGraphics()->getCurrentDevice();

    device->vkDevice().destroySemaphore(_STD exchange(_tmpSemaphore, nullptr));

    for (auto&& cascadeFramebuffer : _framebuffer) {
        if (not cascadeFramebuffer.empty()) {
            cascadeFramebuffer->destroy();
            cascadeFramebuffer.reset();
        }
    }

    for (auto&& cascadeView : _dirShadowCascades) {
        cascadeView.reset();
    }
    _dirShadowMap.reset();

    _sceneLightView.reset();
    _sceneShadowView.reset();
    _staticInstanceView.reset();

    _compiled.pipeline.reset();
    _pass.reset();
    _effect.reset();
}

void TmpDirectionalShadowPass::declareOutputs(ref<graph::ScopedSymbolContext> symCtx_) noexcept {
    symCtx_.registerExposeSymbol(makeDirectionalShadowSymbol(), &_resources.outDirShadows);
}

void TmpDirectionalShadowPass::declareInputs(ref<graph::ScopedSymbolContext> symCtx_) noexcept {
    symCtx_.registerImportSymbol(makeSceneViewSymbol(), &_resources.inSceneView);
}

void TmpDirectionalShadowPass::iterate(cref<graph::ScopedSymbolContext> symCtx_) noexcept {

    if (_dirShadowMap.empty()) {
        setup(const_cast<ref<graph::ScopedSymbolContext>>(symCtx_));
    }

    if (_effect.empty()) {
        _effect = build_test_effect();
    }

    if (_pass.empty()) {
        _pass = build_test_pass();
    }

    /**/

    auto sceneViewRes = symCtx_.getImportSymbol(makeSceneViewSymbol());
    auto sceneView = sceneViewRes->load<smr<const gfx::scene::SceneView>>();

    const auto sys = sceneView->getRenderSceneSystem();
    sys->getRegistry().forEach<gfx::scene::DirectionalLightModel>(
        [this](const auto& model_) {
            _dirLightModel = std::addressof(model_);
        }
    );
}

void TmpDirectionalShadowPass::resolve() noexcept {
    if (_effect && _compiled.flag == accel::EffectCompileResultFlag::eUnknown) {
        _compiled = build_test_pipeline(clone(_effect->effect), clone(_pass));
    }
}

void TmpDirectionalShadowPass::execute(cref<graph::ScopedSymbolContext> symCtx_) noexcept {

    const_cast<ref<graph::ScopedSymbolContext>>(symCtx_).eraseExposedSymbol(makeDirectionalShadowSymbol());
    const_cast<ref<graph::ScopedSymbolContext>>(symCtx_).exposeSymbol(
        makeDirectionalShadowSymbol(),
        _resources.outDirShadows.obj()
    );

    /**/

    if (_dirLightModel == nullptr) {
        return;
    }

    assert(_effect);
    assert(_compiled.pipeline);

    /**/

    auto sceneViewRes = symCtx_.getImportSymbol(makeSceneViewSymbol());
    auto sceneView = sceneViewRes->load<smr<gfx::scene::SceneView>>();

    /**/

    if (_sceneShadowView == nullptr) {
        _sceneShadowView = make_uptr<StorageBufferView>();
    }

    const auto sceneSystem = sceneView->getRenderSceneSystem();
    const auto scenePool = sceneSystem->getSceneResourcePool();
    const auto& shadowSourcePool = scenePool->shadowSourcePool;

    _sceneShadowView->storeBuffer(shadowSourcePool.getPoolView());

    /**/

    {
        static float lambda = 0.95F;

        /**/

        constexpr float ZNEAR = 0.5F;
        constexpr float ZFAR = 48.F;
        const auto clipRange = _STD abs(ZFAR - ZNEAR);

        const auto minZ = ZNEAR;
        const auto maxZ = ZNEAR + clipRange;
        const auto ratio = maxZ / minZ;

        f32 splits[cascadeCount] {};

        for (auto i = 0u; i < cascadeCount; ++i) {

            const auto p = (i + 1u) / static_cast<f32>(cascadeCount);
            const auto log = minZ * _STD powf(ratio, p);
            const auto uniform = minZ + clipRange * p;
            const auto d = lambda * (log - uniform) + uniform;

            splits[i] = (d - ZNEAR) / clipRange;
        }

        /**/

        auto camProj = sceneView->getProjectionMatrix();
        camProj[2][2] *= -1.F;
        const auto invCamVp = (sceneView->getProjectionMatrix() * sceneView->getViewMatrix()).inverse();

        /**/

        GlslDirectionalShadow storeShadow {
            {
                math::mat4::make_identity(), math::mat4::make_identity(),
                math::mat4::make_identity(), math::mat4::make_identity()
            },
            { 0.F, 0.F, 0.F, 0.F }
        };

        /**/

        auto splitDist = splits[0u];
        auto prevSplitDist = 0.F;

        for (auto c = 0u; c < cascadeCount; ++c) {

            math::vec3 orthoFrustum[8] = {
                math::vec3 { -1.F, 1.F, 0.F },
                math::vec3 { 1.F, 1.F, 0.F },
                math::vec3 { 1.F, -1.F, 0.F },
                math::vec3 { -1.F, -1.F, 0.F },
                math::vec3 { -1.F, 1.F, 1.F },
                math::vec3 { 1.F, 1.F, 1.F },
                math::vec3 { 1.F, -1.F, 1.F },
                math::vec3 { -1.F, -1.F, 1.F }
            };

            for (auto i = 0u; i < 8u; ++i) {
                const auto invCorner = invCamVp * math::vec4(orthoFrustum[i], 1.F);
                orthoFrustum[i] = invCorner / invCorner.w;
            }

            for (auto i = 0u; i < 4u; ++i) {
                const auto dist = orthoFrustum[i + 4u] - orthoFrustum[i];
                orthoFrustum[i + 4u] = orthoFrustum[i] + (dist * splitDist);
                orthoFrustum[i] = orthoFrustum[i] + (dist * prevSplitDist);
            }

            math::vec3 orthoCenter {};
            for (auto i = 0u; i < 8u; ++i) {
                orthoCenter += (orthoFrustum[i] / 8.F);
            }

            auto radius = 0.F;
            for (auto i = 0u; i < 8u; ++i) {
                const auto dist = (orthoFrustum[i] - orthoCenter).length();
                radius = MAX(radius, dist);
            }
            radius = _STD ceilf(radius * 16.F) / 16.F;

            const auto maxExt = math::fvec3 { radius };
            const auto minExt = -maxExt;

            /**/

            //const auto origin = sceneView->getOrigin();
            const auto origin = math::Location { 2.24934125F, 8.69429016F, 6.70459509F };
            //const auto lightDirection = (-origin.fvec3()).normalized();

            //const auto lightDirection = math::fvec3 { -0.218508005F, -0.707106829F, -0.672498584F }.normalized();
            const auto lightDirection = _dirLightModel->getLightDirection();

            //const auto castOrigin = origin.fvec3() + lightDirection * (-1.F * -1.F) * distance;
            const auto castOrigin = orthoCenter - lightDirection * -minExt.z;
            // const auto castTarget = origin;
            const auto castTarget = orthoCenter;

            //auto projection = math::perspective(glm::radians(45.F), 1.F, .5F, distance);
            auto projection = math::ortho(minExt.x, maxExt.x, minExt.y, maxExt.y, 0.F, maxExt.z - minExt.z);
            const auto view = math::lookAt(
                castOrigin,
                castTarget,
                (lightDirection - math::vec3_down).zero() ? math::vec3_right : math::vec3_up
            );

            /**/

            storeShadow.shadowViewProj[c] = projection * view;
            storeShadow.cascadeDepth[c] = (ZNEAR + splitDist * clipRange) * -1.F;

            prevSplitDist = splitDist;
            splitDist = splits[MIN(c + 1u, cascadeCount - 1u)];
        }

        /**/

        auto dataView = shadowSourcePool.getDataView(_dirLightModel->_sceneShadowIndex);
        auto* const page = dataView->pages().front();
        auto allocated = page->memory()->allocated();

        allocated->map(allocated->size);
        const auto innerOffset = dataView->offset() - page->resourceOffset();
        memcpy(static_cast<ptr<char>>(allocated->mapping) + innerOffset, &storeShadow, sizeof(storeShadow));
        allocated->flush(VK_WHOLE_SIZE);
        allocated->unmap();
    }

    /**/

    if (_staticInstanceView == nullptr) {
        _staticInstanceView = make_uptr<gfx::StorageBufferView>();
    }

    const auto& staticInstancePool = scenePool->staticInstancePool;
    _staticInstanceView->storeBuffer(staticInstancePool.getPoolView());

    /**/

    if (_framebuffer[0].empty()) {

        for (auto i = 0u; i < cascadeCount; ++i) {

            _framebuffer[i] = make_smr<Framebuffer>(Engine::getEngine()->getGraphics()->getCurrentDevice());
            auto& fb = *_framebuffer[i];

            fb.addAttachment(clone(_dirShadowCascades[i]).into<TextureLikeObject>());
            fb.setExtent(_dirShadowCascades[i]->extent());
            fb.setLayer(1uL);

            fb.setRenderPass(clone(_pass));
            fb.setup();
        }

    }

    if (_tmpSemaphore == nullptr) {
        _tmpSemaphore = _framebuffer[0]->device()->vkDevice().createSemaphore({});
    }

    /**/

    auto cmd = cmd::RenderCommandBuffer {};
    cmd.begin();

    cmd.lambda(
        [_dirShadowMap = clone(_dirShadowMap)](ref<AccelCommandBuffer> cmd_) -> void {

            const auto barrier = vk::ImageMemoryBarrier {
                vk::AccessFlags {},
                vk::AccessFlagBits::eDepthStencilAttachmentWrite,
                vk::ImageLayout::eUndefined,
                vk::ImageLayout::eDepthAttachmentOptimal,
                VK_QUEUE_FAMILY_IGNORED,
                VK_QUEUE_FAMILY_IGNORED,
                _dirShadowMap->buffer().image(),
                vk::ImageSubresourceRange {
                    vk::ImageAspectFlagBits::eDepth,
                    0uL,
                    _dirShadowMap->mipLevels(),
                    0uL,
                    _dirShadowMap->layer()
                }
            };

            cmd_.vkCommandBuffer().pipelineBarrier(
                vk::PipelineStageFlagBits::eAllCommands,
                vk::PipelineStageFlagBits::eAllCommands,
                vk::DependencyFlagBits::eByRegion,
                0uL,
                nullptr,
                0uL,
                nullptr,
                1uL,
                &barrier
            );
        }
    );

    for (auto cascade = 0u; cascade < cascadeCount; ++cascade) {

        cmd.beginAccelPass({ _pass.get(), _framebuffer[cascade].get(), { vk::ClearDepthStencilValue { 1.F, 0uL } } });
        cmd.beginSubPass();

        auto gpipe = clone(_compiled.pipeline).into<GraphicsPipeline>();
        for (const auto& group : gpipe->getBindingLayout().groups) {
            for (const auto& element : group.elements) {

                const auto symbolId = element.symbol->symbolId;
                const auto aliasId = _compiled.alias.aliasOrValue(symbolId);

                const auto iter = _STD ranges::find(
                    _effect->pattern->st.pairs,
                    aliasId,
                    [](const auto& pair_) {
                        return pair_.second;
                    }
                );

                if (iter != _effect->pattern->st.pairs.end()) {
                    assert(false);
                }

                static auto viewSym = lang::SymbolId::from("view"sv);
                if (aliasId == viewSym) {
                    cmd.bindStorage(clone(symbolId), _sceneShadowView.get());
                    continue;
                }

                static auto modelSym = lang::SymbolId::from("model"sv);
                if (aliasId == modelSym) {
                    cmd.bindStorage(clone(symbolId), _staticInstanceView.get());
                    continue;
                }

                __debugbreak();
            }

        }

        cmd.bindGraphicsPipeline(clone(gpipe));

        cmd.lambda(
            [cascade, gpipe](ref<AccelCommandBuffer> cmd_) {
                struct PushBlock {
                    u32 cascade;
                };

                auto block = PushBlock { .cascade = cascade };
                cmd_.vkCommandBuffer().pushConstants(
                    reinterpret_cast<VkPipelineLayout>(Cast<VkGraphicsPipeline>(gpipe.get())->_vkPipeLayout),
                    vk::ShaderStageFlagBits::eVertex,
                    0uL,
                    sizeof(PushBlock),
                    &block
                );
            }
        );

        const auto sys = sceneView->getRenderSceneSystem();
        sys->getRegistry().forEach<gfx::scene::StaticGeometryModel>(
            [&cmd](const gfx::scene::StaticGeometryModel& model_) {

                const auto& modelResource = model_.geometryResource();
                const auto modelGuard = modelResource->acquire(resource::ResourceUsageFlag::eRead);

                const auto& indexBuffer = modelGuard->indices();
                const auto& vertexBuffer = modelGuard->vertices();

                cmd.lambda(
                    [&indexBuffer, &vertexBuffer](ref<AccelCommandBuffer> cmd_) {
                        cmd_.bindIndexBuffer(indexBuffer.get());
                        cmd_.bindVertexBuffer(0uL, vertexBuffer.get());
                    }
                );

                cmd.drawStaticMesh(
                    1uL,
                    model_._sceneInstanceIndex,
                    indexBuffer->size() / sizeof(u32) / 3uLL,
                    0uL
                );
            }
        );

        cmd.endSubPass();
        cmd.endAccelPass();

    }

    cmd.lambda(
        [_dirShadowMap = clone(_dirShadowMap)](ref<AccelCommandBuffer> cmd_) -> void {

            const auto barrier = vk::ImageMemoryBarrier {
                vk::AccessFlagBits::eDepthStencilAttachmentWrite,
                vk::AccessFlagBits::eShaderRead,
                vk::ImageLayout::eDepthAttachmentOptimal,
                vk::ImageLayout::eDepthReadOnlyOptimal,
                VK_QUEUE_FAMILY_IGNORED,
                VK_QUEUE_FAMILY_IGNORED,
                _dirShadowMap->buffer().image(),
                vk::ImageSubresourceRange {
                    vk::ImageAspectFlagBits::eDepth,
                    0uL,
                    _dirShadowMap->mipLevels(),
                    0uL,
                    _dirShadowMap->layer()
                }
            };

            cmd_.vkCommandBuffer().pipelineBarrier(
                vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eFragmentShader,
                vk::PipelineStageFlagBits::eFragmentShader,
                vk::DependencyFlagBits::eByRegion,
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
            reinterpret_cast<Vector<VkSemaphore>&>(_resources.outDirShadows->barriers)
        );

        for (auto i = batch->_tmpWaitFlags.size(); i < batch->_tmpWaits.size(); ++i) {
            batch->_tmpWaitFlags.emplace_back(VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT);
        }

        _resources.outDirShadows->barriers.clear();

        /**/

        batch->_tmpSignals.emplace_back(_tmpSemaphore);
        _resources.outDirShadows->barriers.clear();
        _resources.outDirShadows->barriers.emplace_back(_tmpSemaphore.operator VkSemaphore());
    }

    batch->commitAndDispose();
    nativeBatch.reset();
}

/**/

#include "__tmp_helper.hpp"

using namespace hg::engine::accel::lang;

smr<material::MaterialEffect> build_test_effect() {

    auto vertexStage = make_smr<Stage>(StageFlagBits::eVertex);
    auto fragmentStage = make_smr<Stage>(StageFlagBits::eFragment);

    /**/

    const auto vertexShaderCode = read_shader_file("__test__shadowDir.vs");
    const auto fragmentShaderCode = read_shader_file("__test__shadowDir.fs");

    vertexStage->setIntermediate(make_smr<lang::Intermediate>());
    fragmentStage->setIntermediate(make_smr<lang::Intermediate>());

    vertexStage->getIntermediate()->lang.dialect = Dialect::eVulkanGlsl460;
    vertexStage->getIntermediate()->lang.text.emplace_back(_STD move(vertexShaderCode));

    fragmentStage->getIntermediate()->lang.dialect = Dialect::eVulkanGlsl460;
    fragmentStage->getIntermediate()->lang.text.emplace_back(_STD move(fragmentShaderCode));

    /**/

    {
        auto var = make_uptr<Variable>();
        var->type = Type { .category = TypeCategory::eScalar, .scalarType = ScalarType::eF32Vec3 };
        var->annotation = make_uptr<SimpleAnnotation<AnnotationType::eForwardLinkage>>();
        var->annotation = make_uptr<SymbolIdAnnotation>("vertex-position", _STD move(var->annotation));
        var->annotation = make_uptr<VkBindLocationAnnotation>(
            -1L,
            0L,
            offsetof(engine::gfx::vertex, position),
            _STD move(var->annotation)
        );

        auto sym = make_uptr<Symbol>(
            SymbolId::from("vertex-position"sv),
            VariableSymbol { SymbolType::eVariableSymbol, var.get() }
        );

        vertexStage->getIntermediate()->rep.globalScope.inbound.emplace_back(_STD move(var));
        vertexStage->getIntermediate()->rep.symbolTable.insert(_STD move(sym));
    }

    /**/

    {
        auto var = make_uptr<Variable>();
        var->type = Type { .category = TypeCategory::eObject, .objectType = ObjectType::eUnknown };
        var->annotation = make_uptr<SimpleAnnotation<AnnotationType::eExternalLinkage>>();
        var->annotation = make_uptr<SimpleAnnotation<AnnotationType::eStorage>>(_STD move(var->annotation));
        var->annotation = make_uptr<SymbolIdAnnotation>("view", _STD move(var->annotation));

        auto sym = make_uptr<Symbol>(
            SymbolId::from("view"sv),
            VariableSymbol { SymbolType::eVariableSymbol, var.get() }
        );

        vertexStage->getIntermediate()->rep.globalScope.inbound.emplace_back(_STD move(var));
        vertexStage->getIntermediate()->rep.symbolTable.insert(_STD move(sym));
    }

    /**/

    {
        auto var = make_uptr<Variable>();
        var->type = Type { .category = TypeCategory::eObject, .objectType = ObjectType::eUnknown };
        var->annotation = make_uptr<SimpleAnnotation<AnnotationType::eExternalLinkage>>();
        var->annotation = make_uptr<SimpleAnnotation<AnnotationType::eStorage>>(_STD move(var->annotation));
        var->annotation = make_uptr<SymbolIdAnnotation>("model", _STD move(var->annotation));

        auto sym = make_uptr<Symbol>(
            SymbolId::from("model"sv),
            VariableSymbol { SymbolType::eVariableSymbol, var.get() }
        );

        vertexStage->getIntermediate()->rep.globalScope.inbound.emplace_back(_STD move(var));
        vertexStage->getIntermediate()->rep.symbolTable.insert(_STD move(sym));
    }

    /**/

    {
        auto var = make_uptr<Variable>();
        auto sym = make_uptr<Symbol>();

        var->type = Type { .category = TypeCategory::eObject, .objectType = ObjectType::eUnknown };
        var->annotation = make_uptr<SimpleAnnotation<AnnotationType::eExternalLinkage>>();
        var->annotation = make_uptr<SimpleAnnotation<AnnotationType::eConstant>>(_STD move(var->annotation));
        var->annotation = make_uptr<SimpleAnnotation<AnnotationType::eReadonly>>(_STD move(var->annotation));
        var->annotation = make_uptr<SymbolIdAnnotation>("block", _STD move(var->annotation));

        sym->symbolId = SymbolId::from("block"sv);
        sym->var.type = SymbolType::eVariableSymbol;
        sym->var.data = var.get();

        vertexStage->getIntermediate()->rep.globalScope.inbound.emplace_back(_STD move(var));
        vertexStage->getIntermediate()->rep.symbolTable.insert(_STD move(sym));
    }

    /**/

    {
        auto var = make_uptr<Variable>();
        var->type = Type { .category = TypeCategory::eScalar, .scalarType = ScalarType::eF32 };
        var->annotation = make_uptr<SimpleAnnotation<AnnotationType::eForwardLinkage>>();
        var->annotation = make_uptr<SymbolIdAnnotation>("depth", _STD move(var->annotation));

        auto sym = make_uptr<Symbol>(
            SymbolId::from("depth"sv),
            VariableSymbol { SymbolType::eVariableSymbol, var.get() }
        );

        fragmentStage->getIntermediate()->rep.globalScope.outbound.emplace_back(_STD move(var));
        fragmentStage->getIntermediate()->rep.symbolTable.insert(_STD move(sym));
    }

    /**/

    Guid guid;
    GuidGenerate(guid);

    auto effect = make_smr<AccelerationEffect>(
        _STD move(guid),
        "test-shadow-dir-effect",
        Vector { _STD move(vertexStage), _STD move(fragmentStage) }
    );

    /**/

    auto usagePattern = make_smr<material::EffectUsagePattern>();

    /**/

    return make_smr<material::MaterialEffect>(_STD move(effect), _STD move(usagePattern));
}

smr<const GraphicsPass> build_test_pass() {
    constexpr auto passFactory = VkAccelerationPassFactory {};
    auto pass = passFactory.buildGraphicsPass(
        { makeSceneDepthSymbol() },
        {}
    );
    return pass.value();
}

EffectCompileResult build_test_pipeline(
    mref<smr<const AccelerationEffect>> effect_,
    mref<smr<const GraphicsPass>> pass_
) {

    auto profile = make_smr<EffectProfile>(
        EffectProfile {
            ._name = "Test-Dir-Shadow-Profile",
            ._definitions = {}
        }
    );

    /**/

    //constexpr static f32 depthBiasConstant = 1.1F;
    //constexpr static f32 depthBiasSlope = 1.25F;
    //constexpr static f32 depthBiasClamp = 0.F;

    auto spec = make_smr<SimpleEffectSpecification>();
    spec->setPassSpec(
        make_uptr<GraphicsPassSpecification>(
            DepthCompareOp::eLessEqual,
            DepthBias {
                //depthBiasConstant,
                //depthBiasSlope,
                //depthBiasClamp
            },
            StencilCompareOp::eNever,
            StencilOp::eKeep,
            StencilOp::eKeep,
            StencilOp::eKeep,
            0uL,
            0uL,
            PrimitiveTopology::eTriangleList,
            FaceCulling::eNone,
            FaceMode::eFill,
            FaceWinding::eCcw,
            pass_.get()
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
