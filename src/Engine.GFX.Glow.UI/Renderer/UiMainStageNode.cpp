#include "UiMainStageNode.hpp"

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

#ifdef _DEBUG
#include <cassert>
#endif

#include <Engine.GFX/Renderer/RenderDataToken.hpp>

#include "Engine.GFX/VkFixedPipeline.hpp"
#include "Engine.GFX/API/VkTranslate.hpp"
#include "Engine.GFX/Shader/Factory.hpp"
#include "Engine.GFX/Shader/PrototypeBinding.hpp"
#include "Engine.GFX/Shader/ShaderStorage.hpp"

using namespace ember::engine::gfx::glow::ui::render;
using namespace ember::engine::gfx::render;
using namespace ember::engine::gfx;
using namespace ember;

struct UiTransformBlock {
    math::vec2 translate;
    math::vec2 scale;
};

UiMainStageNode::UiMainStageNode() :
    RenderStageNode() {}

UiMainStageNode::~UiMainStageNode() = default;

void UiMainStageNode::setup(cref<sptr<Device>> device_) {

    SCOPED_STOPWATCH

    /**
     * Store device
     */
    _device = device_;

    /**
     *
     */
    setupLORenderPass();

    /**
     *
     */
    setupShader();

    /**
     *
     */
    setupPipeline();
}

void UiMainStageNode::destroy() {

    SCOPED_STOPWATCH

    /**
     *
     */
    if (_pipeline) {
        _pipeline->destroy();
        _pipeline.reset();
    }

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

    /**
     *
     */
    destroyLORenderPass();
}

bool UiMainStageNode::allocate(const ptr<gfx::render::HORenderPass> renderPass_) {
    return true;
}

bool UiMainStageNode::free(const ptr<gfx::render::HORenderPass> renderPass_) {
    return true;
}

Vector<RenderDataToken> UiMainStageNode::requiredToken() noexcept {
    return {};
}

Vector<RenderDataToken> UiMainStageNode::optionalToken() noexcept {
    return {};
}

void UiMainStageNode::before(const non_owning_rptr<gfx::render::HORenderPass> renderPass_,
    const non_owning_rptr<gfx::render::RenderStagePass> stagePass_) const {
    RenderStageNode::before(renderPass_, stagePass_);
}

void UiMainStageNode::invoke(const non_owning_rptr<gfx::render::HORenderPass> renderPass_,
    const non_owning_rptr<gfx::render::RenderStagePass> stagePass_,
    const non_owning_rptr<SceneNodeModel> model_) const {
    RenderStageNode::invoke(renderPass_, stagePass_, model_);
}

void UiMainStageNode::after(const non_owning_rptr<gfx::render::HORenderPass> renderPass_,
    const non_owning_rptr<gfx::render::RenderStagePass> stagePass_) const {
    RenderStageNode::after(renderPass_, stagePass_);
}

void UiMainStageNode::setupShader() {

    /**
     *
     */
    shader::Factory shaderFactory { _device };
    ShaderStorage& shaderStorage { ShaderStorage::get() };

    /**
     * Prepare Prototype Bindings
     */
    shader::PrototypeBinding textureAtlas {
        shader::BindingType::eImageSampler,
        1ui32,
        shader::BindingUpdateInterval::ePerInstance,
        "uiPassTextureAtlas"
    };

    /**
     * Prepare Prototype Shader
     */
    shader::Prototype vertexPrototype { shader::ShaderType::eVertex, "uiMainPass" };

    auto vertexShaderCode { read_shader_file("resources/shader/uipass.vert.spv") };
    vertexPrototype.storeCode(vertexShaderCode.data(), vertexShaderCode.size());

    shader::Prototype fragmentPrototype { shader::ShaderType::eFragment, "uiMainPass" };

    auto fragmentShaderCode { read_shader_file("resources/shader/uipass.frag.spv") };
    fragmentPrototype.storeCode(fragmentShaderCode.data(), fragmentShaderCode.size());
    fragmentPrototype.add(textureAtlas);

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

void UiMainStageNode::setupPipeline() {

    assert(!_pipeline);

    _pipeline = make_sptr<VkFixedPipeline>(_device, _loRenderPass);
    assert(_pipeline);

    _pipeline->topology() = PrimitiveTopology::eTriangleList;
    _pipeline->viewport() = Viewport {};

    _pipeline->inputs().push_back(FixedPipelineInput {
        0ui8,
        InputRate::ePerVertex,
        sizeof(vertex),
        Vector<InputAttribute> {
            { 0ui32, TextureFormat::eR32G32B32Sfloat, static_cast<u32>(offsetof(vertex, position)) },
            { 1ui32, TextureFormat::eR8G8B8Unorm, static_cast<u32>(offsetof(vertex, color)) },
            { 2ui32, TextureFormat::eR32G32B32Sfloat, static_cast<u32>(offsetof(vertex, uvm)) },
            { 3ui32, TextureFormat::eR32G32B32Sfloat, static_cast<u32>(offsetof(vertex, normal)) }
        }
    });

    _pipeline->vertexStage().shaderSlot().name() = "uiMainPass";
    _pipeline->fragmentStage().shaderSlot().name() = "uiMainPass";

    _pipeline->rasterizationStage().cullFace() = RasterCullFace::eNone;
    // Warning: Unsure whether we should drop depth check support cause of ui component override
    _pipeline->rasterizationStage().depthCheck() = false;

    auto& blend { static_cast<ptr<VkFixedPipeline>>(_pipeline.get())->blending() };
    const vk::PipelineColorBlendAttachmentState colorState {
        VK_TRUE,
        vk::BlendFactor::eSrcAlpha,
        vk::BlendFactor::eOneMinusSrcAlpha,
        vk::BlendOp::eAdd,
        vk::BlendFactor::eOne,
        vk::BlendFactor::eZero,
        vk::BlendOp::eAdd,
        vk::ColorComponentFlagBits::eR |
        vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB |
        vk::ColorComponentFlagBits::eA
    };

    // Push color blending for output color
    blend.push_back(colorState);

    static_cast<ptr<VkFixedPipeline>>(_pipeline.get())->pushConstants().push_back({
        vk::ShaderStageFlagBits::eVertex,
        0ui32,
        sizeof(UiTransformBlock)
    });

    /**
     *
     */
    _pipeline->setup();
}

void UiMainStageNode::setupLORenderPass() {

    /**
     *
     */
    _loRenderPass = make_sptr<pipeline::LORenderPass>(_device);

    // Color Attachment :: Used to store composed color
    _loRenderPass->set(0, vk::AttachmentDescription {
        vk::AttachmentDescriptionFlags(),
        vk::Format::eB8G8R8A8Unorm,
        vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eClear,
        vk::AttachmentStoreOp::eStore,
        vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::ePresentSrcKHR
    });

    /**
     *
     */
    _loRenderPass->setup();
}

void UiMainStageNode::destroyLORenderPass() {

    if (_loRenderPass) {
        _loRenderPass->destroy();
        _loRenderPass.reset();
    }

}
