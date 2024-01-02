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
#include <Engine.Scene/IRenderScene.hpp>
#include <Engine.Accel.Pipeline/GraphicsPipeline.hpp>
#include <Engine.Common/Math/Coordinates.hpp>
#include <Engine.Driver.Vulkan/VkRCmdTranslator.hpp>
#include <Engine.GFX.Render.Predefined/Symbols/SceneDepth.hpp>
#include <Engine.GFX.RenderGraph/Relation/TextureDescription.hpp>
#include <Engine.GFX/Buffer/BufferFactory.hpp>
#include <Engine.GFX/Pool/SceneResourcePool.hpp>

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
                .extent = math::uivec3 { 512uL, 512uL, 1uL },
                .format = TextureFormat::eD32Sfloat,
                .mipLevels = 1uL,
                .type = TextureType::e2d,
                .vkAspect = vk::ImageAspectFlagBits::eDepth,
                .vkUsage = vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled,
                .vkMemoryFlags = vk::MemoryPropertyFlagBits::eDeviceLocal,
                .vkSharing = vk::SharingMode::eExclusive
            }
        )
    );
    _dirShadowMap->buffer()._vkLayout = vk::ImageLayout::eDepthReadOnlyOptimal;
    textureFactory->buildView(*_dirShadowMap, {});

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

    _framebuffer->destroy();
    _framebuffer.reset();

    _dirShadowMap->destroy();
    _dirShadowMap.reset();

    _lightViewBuffer.destroy();

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

    _batchGeometry.clear();
    _batchGeometry.reserve(128uLL);

    auto sceneViewRes = symCtx_.getImportSymbol(makeSceneViewSymbol());
    auto sceneView = sceneViewRes->load<smr<const gfx::scene::SceneView>>();

    auto scene = sceneView->getScene()->renderGraph();
    scene->traversal(
        [this](const ptr<scene::SceneGraph<gfx::scene::SceneModel>::node_type> node_) -> bool {
            auto size = node_->exclusiveSize();
            for (decltype(size) i = 0; i < size; ++i) {

                const auto* const el = node_->elements()[i];
                if (ExactType<gfx::scene::StaticGeometryModel>(*el)) {
                    _batchGeometry.emplace_back(static_cast<const ptr<const gfx::scene::StaticGeometryModel>>(el));

                } else if (ExactType<gfx::scene::DirectionalLightModel>(*el)) {
                    _dirLightModel = static_cast<const ptr<const gfx::scene::DirectionalLightModel>>(el);
                }

            }
            return true;
        }
    );
}

void TmpDirectionalShadowPass::resolve() noexcept {
    if (_effect && _compiled.flag == accel::EffectCompileResultFlag::eUnknown) {
        _compiled = build_test_pipeline(clone(_effect->effect), clone(_pass));
    }
}

void TmpDirectionalShadowPass::execute(cref<graph::ScopedSymbolContext> symCtx_) noexcept {

    if (_dirLightModel == nullptr) {
        return;
    }

    assert(_effect);
    assert(_compiled.pipeline);

    /**/

    const_cast<ref<graph::ScopedSymbolContext>>(symCtx_).eraseExposedSymbol(makeDirectionalShadowSymbol());
    const_cast<ref<graph::ScopedSymbolContext>>(symCtx_).exposeSymbol(
        makeDirectionalShadowSymbol(),
        _resources.outDirShadows.obj()
    );

    /**/

    auto sceneViewRes = symCtx_.getImportSymbol(makeSceneViewSymbol());
    auto sceneView = sceneViewRes->load<smr<gfx::scene::SceneView>>();

    /**/

    if (_lightViewBuffer.buffer == nullptr) {

        auto bufferFactory = BufferFactory::get();

        auto buffer = bufferFactory->build(
            {
                sizeof(math::mat4),
                alignof(float),
                MemoryProperty::eHostVisible,
                vk::BufferCreateFlags {},
                vk::BufferUsageFlagBits::eUniformBuffer,
                vk::SharingMode::eExclusive
            }
        );

        _lightViewBuffer = _STD move(buffer);
    }

    static float distance = 15.F;
    const auto origin = sceneView->getOrigin();
    const auto lightDirection = _dirLightModel->getLightDirection() + math::fvec3(0.000001, 0.F, 0.000001);
    const auto castOrigin = origin.fvec3() + lightDirection * -1.F * distance;

    auto projection = math::perspective(glm::radians(45.F), 1.F, 0.001F, _STD sqrtf(2.0) * distance);
    projection[1][1] *= -1.F;

    const auto view = math::lookAt(
        castOrigin,
        origin.fvec3(),
        math::vec3_up
    );

    const auto vp = projection * view;
    _lightViewBuffer.write<math::mat4>(&vp, 1uL);

    if (_lightView == nullptr) {
        _lightView = make_uptr<UniformBufferView>();
        _lightView->storeBuffer(&_lightViewBuffer);
    }

    /**/

    if (_staticInstanceView == nullptr) {
        _staticInstanceView = make_uptr<gfx::StorageBufferView>();
    }

    const auto& staticInstancePool = sceneView->getScene()->getSceneResourcePool()->staticInstancePool;
    _staticInstanceView->storeBuffer(staticInstancePool.getPoolView());

    /**/

    if (_framebuffer.empty()) {

        _framebuffer = make_smr<Framebuffer>(Engine::getEngine()->getGraphics()->getCurrentDevice());

        _framebuffer->addAttachment(clone(_dirShadowMap));
        _framebuffer->setExtent(_dirShadowMap->extent());
        _framebuffer->setLayer(1uL);

        _framebuffer->setRenderPass(clone(_pass));
        _framebuffer->setup();

        _tmpSemaphore = _framebuffer->device()->vkDevice().createSemaphore({});
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

    cmd.beginAccelPass({ _pass.get(), _framebuffer.get(), { vk::ClearDepthStencilValue { 1.F, 0uL } } });
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

            static auto viewSym = lang::SymbolId::from("view");
            if (aliasId == viewSym) {
                cmd.bindUniform(clone(symbolId), _lightView.get());
                continue;
            }

            static auto modelSym = lang::SymbolId::from("model");
            if (aliasId == modelSym) {
                cmd.bindStorage(clone(symbolId), _staticInstanceView.get());
                continue;
            }

            __debugbreak();
        }

    }

    cmd.bindGraphicsPipeline(_STD move(gpipe));

    for (const auto& batch : _batchGeometry) {

        const auto& model = batch;
        const auto& modelResource = model->geometryResource();
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
            model->_sceneInstanceIndex,
            indexBuffer->size() / sizeof(u32) / 3uLL,
            0uL
        );
    }

    cmd.endSubPass();
    cmd.endAccelPass();

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
            SymbolId::from("vertex-position"),
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
        var->annotation = make_uptr<SimpleAnnotation<AnnotationType::eUniform>>(_STD move(var->annotation));
        var->annotation = make_uptr<SymbolIdAnnotation>("view", _STD move(var->annotation));

        auto sym = make_uptr<Symbol>(
            SymbolId::from("view"),
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
            SymbolId::from("model"),
            VariableSymbol { SymbolType::eVariableSymbol, var.get() }
        );

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
            SymbolId::from("depth"),
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

    auto spec = make_smr<SimpleEffectSpecification>();
    spec->setPassSpec(
        make_uptr<GraphicsPassSpecification>(
            DepthCompareOp::eLessEqual,
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
