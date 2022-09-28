#include "RevEarlyBrdfLutNode.hpp"

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

#ifdef _DEBUG
#include <cassert>
#endif

#include <Engine.GFX/VkFixedPipeline.hpp>
#include <Engine.GFX/API/VkTranslate.hpp>
#include <Engine.GFX/Memory/VkAllocator.hpp>
#include <Engine.GFX/Renderer/HORenderPass.hpp>
#include <Engine.GFX/Renderer/RenderPassState.hpp>
#include <Engine.GFX/Renderer/RenderStagePass.hpp>
#include <Engine.GFX/Shader/DiscreteBindingGroup.hpp>
#include <Engine.GFX/Shader/Factory.hpp>
#include <Engine.GFX/Shader/Prototype.hpp>
#include <Engine.GFX/Shader/PrototypeBinding.hpp>
#include <Engine.GFX/Shader/ShaderStorage.hpp>
#include <Engine.GFX/Renderer/RenderDataToken.hpp>

#include "__macro.hpp"
#include "RevDepthSharedNode.hpp"
#include "Ember/StaticGeometryComponent.hpp"
#include "Engine.Common/Math/Coordinates.hpp"
#include "Engine.GFX/Resource/StaticGeometryResource.hpp"
#include "Engine.GFX/Scene/StaticGeometryModel.hpp"
#include "Engine.Reflect/EmberReflect.hpp"
#include <Engine.GFX/Scene/StaticGeometryBatch.hpp>
#include <Engine.GFX/Rev/Texture/RevVirtualMarkerTexture.hpp>

#include "Engine.GFX/Texture/TextureFactory.hpp"
#include "Engine.GFX/Texture/Texture.hpp"

using namespace ember::engine::gfx::render;
using namespace ember;

RevEarlyBrdfLutNode::RevEarlyBrdfLutNode() :
    RenderStageNode(),
    _brdfLutExtent(512ui32, 512ui32) {}

void RevEarlyBrdfLutNode::setup(cref<sptr<Device>> device_) {

    SCOPED_STOPWATCH

    /**
     * Store device
     */
    _device = device_;

    /**
     *
     */
    auto start { _STD chrono::high_resolution_clock::now() };

    /**
     *
     */
    setupShader(device_);

    // LORenderPass
    auto lorp { setupLORenderPass() };

    /**
     * Fixed Pipeline
     */
    auto pipeline = make_sptr<VkFixedPipeline>(device_, lorp);
    assert(pipeline);

    pipeline->topology() = PrimitiveTopology::eTriangleList;
    pipeline->viewport() = Viewport {
        0ui32,
        0ui32,
        0.F,
        1.F
    };

    pipeline->vertexStage().shaderSlot().name() = "brdfLutGen";
    pipeline->fragmentStage().shaderSlot().name() = "brdfLutGen";

    pipeline->rasterizationStage().cullFace() = RasterCullFace::eNone;
    pipeline->rasterizationStage().depthWrite() = false;

    auto& blending { static_cast<ptr<VkFixedPipeline>>(pipeline.get())->blending() };
    const vk::PipelineColorBlendAttachmentState colorState {
        VK_FALSE,
        vk::BlendFactor::eOne,
        vk::BlendFactor::eZero,
        vk::BlendOp::eAdd,
        vk::BlendFactor::eOne,
        vk::BlendFactor::eZero,
        vk::BlendOp::eAdd,
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
        vk::ColorComponentFlagBits::eA
    };

    blending.push_back(colorState);

    /**
     *
     */
    pipeline->setup();

    /**
     * Framebuffer
     */
    auto framebuffer { setupFramebuffer(lorp) };

    /**
     * Allocate Command Buffer
     */
    auto queue { _device->graphicsQueue() };
    auto pool = queue->pool();

    pool->lck().acquire();

    auto cmd { pool->make() };

    /**
     * Dispatch Render Pass
     */
    cmd.begin();
    cmd.beginRenderPass(*lorp, framebuffer);

    cmd.bindPipeline(pipeline.get(), Viewport { framebuffer.width(), framebuffer.height() });

    cmd.draw(1, 0, 6, 0);

    cmd.endRenderPass();
    cmd.end();

    /**
     * Submit to queue
     */
    queue->submitWait(cmd);

    /**
     *
     */
    cmd.release();

    pool->lck().release();

    /**
     * Extract texture and release framebuffer
     */
    _brdfLut = framebuffer.attachments().front().unwrapped();
    destroyFramebuffer(_STD move(framebuffer));

    /**
     *
     */
    if (pipeline) {
        pipeline->destroy();
        pipeline.reset();
    }

    // Destroy LORenderPass
    destroyLORenderPass(_STD move(lorp.get()));
    lorp.reset();

    /**
     *
     */
    auto end { _STD chrono::high_resolution_clock::now() };

    _STD stringstream ss {};
    ss << "Generating BRDF LUT took "sv;
    ss << (_STD chrono::duration_cast<_STD chrono::milliseconds>(end - start).count());
    ss << " ms"sv;

    DEBUG_SNMSG(true, "LOG", ss.str());
}

void RevEarlyBrdfLutNode::destroy() {

    SCOPED_STOPWATCH

    /**
     *
     */
    for (auto& entry : _requiredDescriptorPools) {
        delete[] entry.pPoolSizes;

        entry.pPoolSizes = nullptr;
        entry.poolSizeCount = 0ui32;
    }

    /**
     *
     */
    for (auto& entry : _requiredBindingGroups) {
        _device->vkDevice().destroyDescriptorSetLayout(entry.vkSetLayout());
    }

}

bool RevEarlyBrdfLutNode::allocate(const ptr<HORenderPass> renderPass_) {

    SCOPED_STOPWATCH

    const auto state { renderPass_->state() };

    /**
     * Store State
     */
    state->data.insert_or_assign("RevEarlyBrdfLutNode::BrdfLut"sv, _brdfLut);

    return true;
}

bool RevEarlyBrdfLutNode::free(const ptr<HORenderPass> renderPass_) {

    SCOPED_STOPWATCH

    const auto state { renderPass_->state() };

    /**
     * Free Brdf
     */
    auto it = state->data.find("RevEarlyBrdfLutNode::BrdfLut"sv);
    if (it != state->data.end()) {
        // First make a typed copy, than erase state entry
        auto brdf { _STD static_pointer_cast<decltype(_brdfLut)::element_type, void>(it->second) };
        state->data.erase(it);
    }

    return true;
}

Vector<RenderDataToken> RevEarlyBrdfLutNode::requiredToken() noexcept {
    return {};
}

Vector<RenderDataToken> RevEarlyBrdfLutNode::optionalToken() noexcept {
    return {};
}

void RevEarlyBrdfLutNode::before(
    const non_owning_rptr<HORenderPass> renderPass_,
    const non_owning_rptr<RenderStagePass> stagePass_
) const {

    SCOPED_STOPWATCH

    const auto& data { renderPass_->state()->data };
}

void RevEarlyBrdfLutNode::invoke(
    const non_owning_rptr<HORenderPass> renderPass_,
    const non_owning_rptr<RenderStagePass> stagePass_,
    const non_owning_rptr<SceneNodeModel> model_
) const {

    SCOPED_STOPWATCH

    auto* state { renderPass_->state().get() };
}

void RevEarlyBrdfLutNode::after(
    const non_owning_rptr<HORenderPass> renderPass_,
    const non_owning_rptr<RenderStagePass> stagePass_
) const {

    SCOPED_STOPWATCH

    const auto& data { renderPass_->state()->data };
}

void RevEarlyBrdfLutNode::setupShader(cref<sptr<Device>> device_) {

    /**
     *
     */
    shader::Factory shaderFactory { device_ };
    ShaderStorage& shaderStorage { ShaderStorage::get() };

    /**
     * Prepare Prototype Shader
     */
    shader::Prototype vertexPrototype { shader::ShaderType::eVertex, "brdfLutGen" };

    auto vertexShaderCode { read_shader_file("resources/shader/brdfLut.gen.vert.spv") };
    vertexPrototype.storeCode(vertexShaderCode.data(), vertexShaderCode.size());

    shader::Prototype fragmentPrototype { shader::ShaderType::eFragment, "brdfLutGen" };

    auto fragmentShaderCode { read_shader_file("resources/shader/brdfLut.gen.frag.spv") };
    fragmentPrototype.storeCode(fragmentShaderCode.data(), fragmentShaderCode.size());

    /**
     * Build Shader and Bindings
     */
    auto factoryResult {
        shaderFactory.build({
            vertexPrototype,
            fragmentPrototype
        })
    };

    /**
     * Store Shader to Storage
     */
    for (const auto& entry : factoryResult.shaders) {
        shaderStorage.store(entry);
    }

    /**
     * Prepare required Descriptor Pools
     */
    for (const auto& group : factoryResult.groups) {

        Vector<vk::DescriptorPoolSize> sizes {};
        for (const auto& binding : group.shaderBindings()) {

            auto it {
                _STD find_if(sizes.begin(), sizes.end(), [type = binding.type()](cref<vk::DescriptorPoolSize> entry_) {
                    return entry_.type == api::vkTranslateBindingType(type);
                })
            };

            if (it == sizes.end()) {
                sizes.push_back({ api::vkTranslateBindingType(binding.type()), 1 });
                continue;
            }

            ++(it->descriptorCount);
        }

        // Persist data structure
        auto* sizeMem { new vk::DescriptorPoolSize[sizes.size()] };
        for (u32 i = 0; i < sizes.size(); ++i) {
            sizeMem[i] = _STD move(sizes[i]);
        }

        const vk::DescriptorPoolCreateInfo dpci {
            vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind,
            1ui32,
            static_cast<u32>(sizes.size()),
            sizeMem
        };

        _requiredDescriptorPools.push_back(dpci);
        _requiredBindingGroups.push_back(group);
    }

}

sptr<engine::gfx::pipeline::LORenderPass> RevEarlyBrdfLutNode::setupLORenderPass() {

    /**
     * LORenderPass
     */
    auto loRenderPass = make_sptr<pipeline::LORenderPass>(_device);

    // Brdf Lut Attachment
    loRenderPass->set(0, vk::AttachmentDescription {
        vk::AttachmentDescriptionFlags(),
        api::vkTranslateFormat(TextureFormat::eR16G16Sfloat),
        vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eClear,
        vk::AttachmentStoreOp::eStore,
        vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eShaderReadOnlyOptimal
    });

    /**
     *
     */
    loRenderPass->setup();
    return loRenderPass;
}

void RevEarlyBrdfLutNode::destroyLORenderPass(mref<ptr<pipeline::LORenderPass>> renderPass_) {}

[[nodiscard]] engine::gfx::Framebuffer RevEarlyBrdfLutNode::setupFramebuffer(
    cref<sptr<pipeline::LORenderPass>> renderPass_) {

    /**
     * Allocate Command Buffer
     */
    auto cmd { _device->graphicsQueue()->pool()->make() };

    /**
     * Create Framebuffer
     */
    const auto* factory { TextureFactory::get() };

    Framebuffer buffer { _device };
    buffer.setExtent(math::uivec3(_brdfLutExtent, 1ui32));
    buffer.setRenderPass(renderPass_);

    /**
     * Create Framebuffer :: Attachments
     */
    auto brdfLut = factory->build({
        buffer.extent(),
        TextureFormat::eR16G16Sfloat,
        // Brdf Lut
        1ui32,
        TextureType::e2d,
        vk::ImageAspectFlagBits::eColor,
        vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        vk::SharingMode::eExclusive
    });

    factory->buildView(brdfLut);

    /**
     * Create Framebuffer :: Attach
     */
    buffer.add({ _STD move(brdfLut) });

    /**
     *
     */
    buffer.setup();

    return buffer;
}

void RevEarlyBrdfLutNode::destroyFramebuffer(mref<Framebuffer> framebuffer_) {
    auto fb { _STD move(framebuffer_) };
    fb.destroy();
}
