#include "EdGridNode.hpp"

#include "Engine.GFX/VkFixedPipeline.hpp"
#include "Engine.GFX.Glow.3D/Renderer/__macro.hpp"
#include "Engine.GFX/API/VkTranslate.hpp"
#include "Engine.GFX/Buffer/Buffer.hpp"
#include "Engine.GFX/Framebuffer/Framebuffer.hpp"
#include "Engine.GFX/Renderer/HORenderPass.hpp"
#include "Engine.GFX/Renderer/RenderPassState.hpp"
#include "Engine.GFX/Texture/TextureFactory.hpp"
#include "Engine.GFX/Command/CommandQueue.hpp"
#include "Engine.GFX/Command/CommandPool.hpp"
#include "Engine.GFX/Command/CommandBuffer.hpp"
#include "Engine.GFX/Renderer/RenderStagePass.hpp"
#include "Engine.GFX/Shader/Factory.hpp"
#include "Engine.GFX/Shader/ShaderStorage.hpp"
#include "Engine.GFX/Renderer/RenderDataToken.hpp"
#include <Engine.GFX.Scene/View/SceneView.hpp>

using namespace hg::editor::gfx;
using namespace hg::engine::gfx::render;
using namespace hg::engine::gfx;
using namespace hg;

EdGridNode::EdGridNode() :
    RenderStageNode() {}

EdGridNode::~EdGridNode() {
    EdGridNode::destroy();
}

void EdGridNode::setup(cref<sptr<Device>> device_) {

    _device = device_;

    setupLORenderPass();
    setupShader();
    setupPipeline();
}

void EdGridNode::destroy() {

    if (_pipeline) {
        _pipeline->destroy();
        _pipeline.reset();
    }

    for (auto& entry : _reqDescPools) {
        delete[] entry.pPoolSizes;

        entry.pPoolSizes = nullptr;
        entry.poolSizeCount = 0ui32;
    }

    for (auto& entry : _reqBindGroups) {
        _device->vkDevice().destroyDescriptorSetLayout(entry.vkSetLayout());
    }

    destroyLORenderPass();
}

bool EdGridNode::allocate(const ptr<HORenderPass> renderPass_) {

    const auto state { renderPass_->state() };

    /**/
    auto cmd { _device->graphicsQueue()->pool()->make() };

    /**/
    const auto* factory { TextureFactory::get() };

    Framebuffer fb { _device };
    fb.setExtent(renderPass_->target()->extent());
    fb.setRenderPass(_loRenderPass);

    /**/

    const ptr<const Framebuffer> depthFrame {
        static_cast<const ptr<const Framebuffer>>(state->data.at("RevDepthStage::Framebuffer"sv).get())
    };

    auto target { renderPass_->target() };
    if (!target->vkView()) {
        factory->buildView(*target);
    }

    fb.add({ target });
    fb.add({ depthFrame->attachments()[0].unwrapped() });
    fb.setup();

    /**/
    Buffer cam {
        nullptr,
        nullptr,
        _device->vkDevice(),
        sizeof(math::mat4) * 2,
        vk::BufferUsageFlagBits::eUniformBuffer
    };

    const vk::BufferCreateInfo bci {
        vk::BufferCreateFlags {},
        cam.size,
        cam.usageFlags,
        vk::SharingMode::eExclusive,
        0,
        nullptr
    };

    cam.buffer = _device->vkDevice().createBuffer(bci);
    assert(cam.buffer);

    auto result {
        memory::allocate(&state->alloc, _device, cam.buffer, MemoryProperty::eHostVisible, cam.memory)
    };
    cam.bind();

    /**/

    const math::mat4 dummy[] { math::mat4::make_identity(), math::mat4::make_identity() };
    cam.write<math::mat4>(&dummy, 2ui32);

    /**/
    Vector<shader::DiscreteBindingGroup> dbgs {};
    Vector<vk::DescriptorPool> pools {};

    for (u64 rdp { 0ui64 }; rdp < _reqDescPools.size(); ++rdp) {

        vk::DescriptorPool pool { _device->vkDevice().createDescriptorPool(_reqDescPools[rdp]) };
        assert(pool);

        const auto& grp { _reqBindGroups[rdp] };

        vk::DescriptorSet set { _device->vkDevice().allocateDescriptorSets({ pool, 1ui32, &grp.vkSetLayout() })[0] };
        assert(set);

        dbgs.push_back(grp.useDiscrete(set));
        pools.push_back(pool);
    }

    /**/
    dbgs[0].getById(shader::ShaderBinding::id_type { 1 }).store(cam);

    /**/
    state->data.insert_or_assign("EdGridNode::CamBuffer"sv, make_sptr<decltype(cam)>(_STD move(cam)));
    state->data.insert_or_assign("EdGridNode::Framebuffer"sv, make_sptr<decltype(fb)>(_STD move(fb)));
    state->data.insert_or_assign("EdGridNode::CommandBuffer"sv, make_sptr<decltype(cmd)>(_STD move(cmd)));
    state->data.insert_or_assign("EdGridNode::DescriptorPools"sv, make_sptr<decltype(pools)>(_STD move(pools)));
    state->data.insert_or_assign("EdGridNode::DiscreteBindingGroups"sv, make_sptr<decltype(dbgs)>(_STD move(dbgs)));

    return true;
}

bool EdGridNode::free(const ptr<HORenderPass> renderPass_) {

    const auto state { renderPass_->state() };

    /**/
    auto it { state->data.find("EdGridNode::DiscreteBindingGroups"sv) };
    if (it != state->data.end()) {

        sptr<Vector<shader::DiscreteBindingGroup>> dbgs {
            _STD static_pointer_cast<Vector<shader::DiscreteBindingGroup>, void>(it->second)
        };

        const auto poolIt { state->data.find("EdGridNode::DescriptorPools"sv) };
        sptr<Vector<vk::DescriptorPool>> pools {
            _STD static_pointer_cast<Vector<vk::DescriptorPool>, void>(poolIt->second)
        };

        /**/
        for (const auto& entry : *pools) {
            _device->vkDevice().destroyDescriptorPool(entry);
        }

        /**/
        state->data.erase(it);
        state->data.erase(poolIt);
    }

    /**/

    it = state->data.find("EdGridNode::CamBuffer"sv);
    if (it != state->data.end()) {

        sptr<Buffer> cam {
            _STD static_pointer_cast<Buffer, void>(it->second)
        };

        cam->destroy();
        state->data.erase(it);
    }

    /**/

    it = state->data.find("EdGridNode::Framebuffer"sv);
    if (it != state->data.end()) {

        sptr<Framebuffer> buffer {
            _STD static_pointer_cast<Framebuffer, void>(it->second)
        };

        buffer->destroy();
        state->data.erase(it);
    }

    /**/

    it = state->data.find("EdGridNode::CommandBuffer"sv);
    if (it != state->data.end()) {

        sptr<CommandBuffer> cmd {
            _STD static_pointer_cast<CommandBuffer, void>(it->second)
        };

        /**/
        if (cmd->vkCommandBuffer()) {
            auto& lck { cmd->lck() };
            lck.acquire();
            cmd->release();
            lck.release();
        }

        /**/
        state->data.erase(it);
    }

    return true;
}

Vector<RenderDataToken> EdGridNode::requiredToken() noexcept {
    return {};
}

Vector<RenderDataToken> EdGridNode::optionalToken() noexcept {
    return {};
}

const non_owning_rptr<const Vector<type_id>> EdGridNode::modelTypes() const noexcept {
    return RenderStageNode::modelTypes();
}

void EdGridNode::before(
    const non_owning_rptr<HORenderPass> renderPass_,
    const non_owning_rptr<RenderStagePass> stagePass_
) const {

    const auto& data { renderPass_->state()->data };

    /**/
    const auto cmdEntry { data.at("EdGridNode::CommandBuffer"sv) };
    auto& cmd { *_STD static_pointer_cast<CommandBuffer, void>(cmdEntry) };
    cmd.begin();

    /**/

    const auto entry { data.at("EdGridNode::Framebuffer"sv) };
    auto& fb { *_STD static_pointer_cast<Framebuffer, void>(entry) };

    /**
     * Update Resources [BindingUpdateInterval::ePerFrame]
     */
    auto& cam { *_STD static_pointer_cast<Buffer, void>(data.at("EdGridNode::CamBuffer"sv)) };

    cref<scene::SceneViewEye> eye { *renderPass_->sceneView() };
    math::mat4 viewData[] { eye.getViewMatrix(), eye.getProjectionMatrix() };
    cam.write<math::mat4>(&viewData, 2ui32);

    /**/
    cmd.beginRenderPass(*_loRenderPass, fb);
    cmd.bindPipeline(
        _pipeline.get(),
        {
            fb.width(),
            fb.height(),
            0.F,
            1.F
        }
    );

    /**/
    sptr<Vector<shader::DiscreteBindingGroup>> dbgs {
        _STD static_pointer_cast<Vector<shader::DiscreteBindingGroup>, void>(
            data.at("EdGridNode::DiscreteBindingGroups"sv)
        )
    };

    for (u32 idx = 0; idx < dbgs->size(); ++idx) {

        const auto& grp { (*dbgs)[idx] };

        if (grp.super().interval() == shader::BindingUpdateInterval::ePerFrame) {
            cmd.bindDescriptor(idx, grp.vkSet());
        }
    }
}

void EdGridNode::invoke(
    const non_owning_rptr<HORenderPass> renderPass_,
    const non_owning_rptr<RenderStagePass> stagePass_,
    const non_owning_rptr<SceneNodeModel> model_
) const {

    const auto& data { renderPass_->state()->data };

    /**/
    auto& cmd { *_STD static_pointer_cast<CommandBuffer, void>(data.at("EdGridNode::CommandBuffer"sv)) };
    cmd.draw(1, 0, 6, 0);
}

void EdGridNode::after(
    const non_owning_rptr<HORenderPass> renderPass_,
    const non_owning_rptr<RenderStagePass> stagePass_
) const {

    const auto& data { renderPass_->state()->data };

    /**/
    auto& cmd { *_STD static_pointer_cast<CommandBuffer, void>(data.at("EdGridNode::CommandBuffer"sv)) };

    /**/
    cmd.endRenderPass();
    cmd.end();

    /**/
    stagePass_->batch().push(cmd);
}

void EdGridNode::setupLORenderPass() {

    _loRenderPass = make_sptr<pipeline::LORenderPass>(_device);

    _loRenderPass->set(
        0,
        vk::AttachmentDescription {
            vk::AttachmentDescriptionFlags(),
            vk::Format::eB8G8R8A8Unorm,
            vk::SampleCountFlagBits::e1,
            vk::AttachmentLoadOp::eLoad,
            vk::AttachmentStoreOp::eStore,
            vk::AttachmentLoadOp::eDontCare,
            vk::AttachmentStoreOp::eDontCare,
            vk::ImageLayout::eShaderReadOnlyOptimal,
            vk::ImageLayout::eShaderReadOnlyOptimal
        }
    );
    _loRenderPass->set(
        1,
        vk::AttachmentDescription {
            vk::AttachmentDescriptionFlags(),
            api::vkTranslateFormat(REV_DEPTH_FORMAT),
            vk::SampleCountFlagBits::e1,
            vk::AttachmentLoadOp::eLoad,
            vk::AttachmentStoreOp::eDontCare,
            vk::AttachmentLoadOp::eDontCare,
            vk::AttachmentStoreOp::eDontCare,
            vk::ImageLayout::eDepthStencilReadOnlyOptimal,
            vk::ImageLayout::eDepthStencilReadOnlyOptimal
        }
    );

    _loRenderPass->setup();
}

void EdGridNode::destroyLORenderPass() {
    if (_loRenderPass) {
        _loRenderPass->destroy();
        _loRenderPass.reset();
    }
}

void EdGridNode::setupShader() {

    shader::Factory shaderFactory { _device };
    ShaderStorage& shaderStorage { ShaderStorage::get() };

    /**/

    shader::PrototypeBinding cam {
        shader::BindingType::eUniformBuffer,
        1,
        shader::BindingUpdateInterval::ePerFrame,
        "edGridCam"
    };

    /**/
    shader::Prototype vertProt { shader::ShaderType::eVertex, "edGridPass" };

    auto vertShaderCode { read_shader_file("resources/shader/edgrid.vert.spv") };
    vertProt.storeCode(vertShaderCode.data(), vertShaderCode.size());
    vertProt.add(cam);

    shader::Prototype fragProt { shader::ShaderType::eFragment, "edGridPass" };

    auto fragShaderCode { read_shader_file("resources/shader/edgrid.frag.spv") };
    fragProt.storeCode(fragShaderCode.data(), fragShaderCode.size());
    fragProt.add(cam);

    /**/

    auto result { shaderFactory.build({ vertProt, fragProt }) };
    for (const auto& entry : result.shaders) {
        shaderStorage.store(entry);
    }

    /**/
    for (const auto& group : result.groups) {

        Vector<vk::DescriptorPoolSize> sizes {};
        for (const auto& binding : group.shaderBindings()) {

            auto it {
                _STD find_if(
                    sizes.begin(),
                    sizes.end(),
                    [type = binding.type()](cref<vk::DescriptorPoolSize> entry_) {
                        return entry_.type == api::vkTranslateBindingType(type);
                    }
                )
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
            vk::DescriptorPoolCreateFlags {},
            1ui32,
            static_cast<u32>(sizes.size()),
            sizeMem
        };

        _reqDescPools.push_back(dpci);
        _reqBindGroups.push_back(group);
    }
}

void EdGridNode::setupPipeline() {

    /**
     * Fixed Pipeline
     */
    _pipeline = make_sptr<VkFixedPipeline>(_device, _loRenderPass);
    assert(_pipeline);

    _pipeline->topology() = PrimitiveTopology::eTriangleList;
    _pipeline->viewport() = Viewport {
        0ui32,
        0ui32,
        0.F,
        1.F
    };

    _pipeline->vertexStage().shaderSlot().name() = "edGridPass";
    _pipeline->fragmentStage().shaderSlot().name() = "edGridPass";

    _pipeline->rasterizationStage().cullFace() = RasterCullFace::eNone;
    _pipeline->rasterizationStage().depthCheck() = true;
    _pipeline->rasterizationStage().depthCompare() = vk::CompareOp::eLess;

    static_cast<ptr<VkFixedPipeline>>(_pipeline.get())->blending().push_back(
        vk::PipelineColorBlendAttachmentState {
            VK_TRUE,
            vk::BlendFactor::eSrcAlpha,
            vk::BlendFactor::eOneMinusSrcAlpha,
            vk::BlendOp::eAdd,
            vk::BlendFactor::eOne,
            vk::BlendFactor::eZero,
            vk::BlendOp::eAdd,
            vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
            vk::ColorComponentFlagBits::eA
        }
    );

    /**
     *
     */
    _pipeline->setup();
}
