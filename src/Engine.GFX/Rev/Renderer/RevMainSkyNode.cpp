#include "RevMainSkyNode.hpp"

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

#ifdef _DEBUG
#include <cassert>
#endif

#include <Engine.GFX/VkFixedPipeline.hpp>
#include <Engine.GFX/API/VkTranslate.hpp>
#include <Engine.GFX/Command/CommandBuffer.hpp>
#include <Engine.GFX/Memory/VkAllocator.hpp>
#include <Engine.GFX/Renderer/HORenderPass.hpp>
#include <Engine.GFX/Renderer/RenderDataToken.hpp>
#include <Engine.GFX/Renderer/RenderPassState.hpp>
#include <Engine.GFX/Renderer/RenderStagePass.hpp>
#include <Engine.GFX/Shader/DiscreteBindingGroup.hpp>
#include <Engine.GFX/Shader/Factory.hpp>
#include <Engine.GFX/Shader/Prototype.hpp>
#include <Engine.GFX/Shader/PrototypeBinding.hpp>
#include <Engine.GFX/Shader/ShaderStorage.hpp>
#include <Engine.Session/Session.hpp>

#include "RevMainSharedNode.hpp"
#include "__macro.hpp"
#include "Engine.GFX/Graphics.hpp"
#include "Engine.GFX/Loader/RevTextureLoader.hpp"
#include "Engine.GFX/Texture/TextureFactory.hpp"

using namespace ember::engine::gfx::render;
using namespace ember::engine::gfx;
using namespace ember;

static Texture testCubeMap {};

RevMainSkyNode::RevMainSkyNode(const ptr<RevMainSharedNode> sharedNode_) :
    _sharedNode(sharedNode_) {}

void RevMainSkyNode::setup(cref<sptr<Device>> device_) {

    SCOPED_STOPWATCH

    /**
     * Store device
     */
    _device = device_;

    /**
     *
     */
    setupShader(device_);

    /**
     * Fixed Pipeline
     */
    _pipeline = make_sptr<VkFixedPipeline>(device_, _sharedNode->loRenderPass());
    assert(_pipeline);

    _pipeline->topology() = PrimitiveTopology::eTriangleList;
    _pipeline->viewport() = Viewport {};

    _pipeline->vertexStage().shaderSlot().name() = "skyMainPass";
    _pipeline->fragmentStage().shaderSlot().name() = "skyMainPass";

    _pipeline->rasterizationStage().cullFace() = RasterCullFace::eFront;
    _pipeline->rasterizationStage().depthWrite() = false;

    auto& blending { static_cast<ptr<VkFixedPipeline>>(_pipeline.get())->blending() };
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

    for (u8 i = 0; i < 4ui8; ++i) {
        blending.push_back(colorState);
    }

    /**
     *
     */
    _pipeline->setup();

    // TODO:
    if (!testCubeMap) {
        RevTextureLoader loader { Session::get()->modules().graphics()->cacheCtrl() };
        testCubeMap = loader.__tmp__load({ ""sv, R"(R:\\sky.ktx)" });

        Vector<vk::ImageMemoryBarrier> imgBarriers {};
        imgBarriers.push_back({
            vk::AccessFlags {},
            vk::AccessFlagBits::eShaderRead,
            vk::ImageLayout::eTransferSrcOptimal,
            vk::ImageLayout::eShaderReadOnlyOptimal,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            testCubeMap.buffer().image(),
            vk::ImageSubresourceRange {
                vk::ImageAspectFlagBits::eColor,
                0,
                testCubeMap.mipLevels(),
                0,
                testCubeMap.layer()
            }
        });

        auto pool = _device->graphicsQueue()->pool();
        pool->lck().acquire();
        CommandBuffer iiCmd = pool->make();
        iiCmd.begin();

        /**
         * Transform
         */
        iiCmd.vkCommandBuffer().pipelineBarrier(vk::PipelineStageFlagBits::eAllCommands,
            vk::PipelineStageFlagBits::eAllCommands, vk::DependencyFlags {},
            0, nullptr,
            0, nullptr,
            static_cast<uint32_t>(imgBarriers.size()), imgBarriers.data()
        );

        iiCmd.end();
        iiCmd.submitWait();
        iiCmd.release();

        pool->lck().release();

        testCubeMap.buffer()._vkLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

        TextureFactory::get()->buildView(testCubeMap);
    }
}

void RevMainSkyNode::destroy() {

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

    // TODO:
    if (testCubeMap) {
        testCubeMap.destroy();
    }

    /**
     *
     */
    for (auto& entry : _requiredBindingGroups) {
        _device->vkDevice().destroyDescriptorSetLayout(entry.vkSetLayout());
    }

}

bool RevMainSkyNode::allocate(const ptr<HORenderPass> renderPass_) {

    SCOPED_STOPWATCH

    const auto state { renderPass_->state() };

    /**
     * Allocate Secondary Command Buffer
     */
    auto cmd { _device->graphicsQueue()->pool()->make(true) };

    /**
     * Allocate Uniform Buffer
     */
    Buffer uniform {
        nullptr,
        nullptr,
        _device->vkDevice(),
        sizeof(math::mat4),
        vk::BufferUsageFlagBits::eUniformBuffer
    };

    const vk::BufferCreateInfo bci {
        vk::BufferCreateFlags {},
        uniform.size,
        uniform.usageFlags,
        vk::SharingMode::eExclusive,
        0ui32,
        nullptr
    };

    uniform.buffer = _device->vkDevice().createBuffer(bci);
    assert(uniform.buffer);

    const auto result {
        memory::allocate(&state->alloc, _device, uniform.buffer, MemoryProperty::eHostVisible, uniform.memory)
    };// TODO: Handle failed allocation
    uniform.bind();

    /**
     * Default insert data
     */
    const ptr<Camera> camera { renderPass_->camera() };
    math::mat4 mv { camera->view() * math::mat4::make_identity() };
    mv[3] = math::vec4(0.F, 0.F, 0.F, 1.F);
    uniform.write<math::mat4>(&mv, 1ui32);

    /**
     * Allocate Descriptors
     */
    Vector<shader::DiscreteBindingGroup> dbgs {};
    Vector<vk::DescriptorPool> pools {};

    for (u64 rdp = 0; rdp < _requiredDescriptorPools.size(); ++rdp) {

        vk::DescriptorPool pool { _device->vkDevice().createDescriptorPool(_requiredDescriptorPools[rdp]) };
        assert(pool);

        const auto& grp { _requiredBindingGroups[rdp] };

        vk::DescriptorSet set { _device->vkDevice().allocateDescriptorSets({ pool, 1ui32, &grp.vkSetLayout() })[0] };
        assert(set);

        dbgs.push_back(grp.useDiscrete(set));
        pools.push_back(pool);
    }

    /**
     * Pre Store
     */
    dbgs[0].getById(shader::ShaderBinding::id_type { 1 }).store(uniform);
    dbgs[1].getById(shader::ShaderBinding::id_type { 2 }).store(testCubeMap);

    /**
     * Store State
     */
    state->data.insert_or_assign("RevMainSkyNode::CommandBuffer"sv,
        _STD make_shared<decltype(cmd)>(_STD move(cmd)));
    state->data.insert_or_assign("RevMainSkyNode::UniformBuffer"sv,
        _STD make_shared<decltype(uniform)>(_STD move(uniform)));
    state->data.insert_or_assign("RevMainSkyNode::DiscreteBindingGroups"sv,
        _STD make_shared<decltype(dbgs)>(_STD move(dbgs)));
    state->data.insert_or_assign("RevMainSkyNode::DescriptorPools"sv,
        _STD make_shared<decltype(pools)>(_STD move(pools)));

    return true;
}

bool RevMainSkyNode::free(const ptr<HORenderPass> renderPass_) {

    SCOPED_STOPWATCH

    const auto state { renderPass_->state() };

    // Warning: Experimental one time recording and reuse
    state->data.erase("RevMainSkyNode::RecordedCommandBuffer"sv);

    /**
     * Free Descriptors
     */
    auto it { state->data.find("RevMainSkyNode::DiscreteBindingGroups"sv) };
    if (it != state->data.end()) {

        sptr<Vector<shader::DiscreteBindingGroup>> dbgs {
            _STD static_pointer_cast<Vector<shader::DiscreteBindingGroup>, void>(it->second)
        };

        const auto poolIt { state->data.find("RevMainSkyNode::DescriptorPools"sv) };
        sptr<Vector<vk::DescriptorPool>> pools {
            _STD static_pointer_cast<Vector<vk::DescriptorPool>, void>(poolIt->second)
        };

        /**
         *
         */
        for (u32 i = 0; i < dbgs->size(); ++i) {

            const auto& dbg { (*dbgs)[i] };
            const auto& pool { (*pools)[i] };

            #ifdef _DEBUG
            //const auto result { device->vkDevice().freeDescriptorSets(pool, 1ui32, &dbg.vkSet()) };
            //assert(result == vk::Result::eSuccess);
            #else
            //_device->vkDevice().freeDescriptorSets(pool, 1ui32, &dbg.vkSet());
            #endif
        }

        /**
         *
         */
        for (const auto& entry : *pools) {
            _device->vkDevice().destroyDescriptorPool(entry);
        }

        /**
         *
         */
        state->data.erase(it);
        state->data.erase(poolIt);
    }

    /**
     * Free Buffers
     */
    it = state->data.find("RevMainSkyNode::UniformBuffer"sv);
    if (it != state->data.end()) {

        sptr<Buffer> uniform {
            _STD static_pointer_cast<Buffer, void>(it->second)
        };

        /**
         *
         */
        uniform->destroy();

        /**
         *
         */
        state->data.erase(it);
    }

    /**
     * Free Command Buffers
     */
    it = renderPass_->state()->data.find("RevMainSkyNode::CommandBuffer"sv);
    if (it != renderPass_->state()->data.end()) {

        sptr<CommandBuffer> cmd {
            _STD static_pointer_cast<CommandBuffer, void>(it->second)
        };

        /**
         *
         */
        if (cmd->vkCommandBuffer()) {
            auto& lck { cmd->lck() };
            lck.acquire();
            cmd->release();
            lck.release();
        }

        /**
         *
         */
        renderPass_->state()->data.erase(it);
    }

    return true;
}

Vector<RenderDataToken> RevMainSkyNode::requiredToken() noexcept {
    return {};
}

Vector<RenderDataToken> RevMainSkyNode::optionalToken() noexcept {
    return {};
}

void RevMainSkyNode::before(
    const non_owning_rptr<HORenderPass> renderPass_,
    const non_owning_rptr<RenderStagePass> stagePass_
) const {

    SCOPED_STOPWATCH

    const auto& data { renderPass_->state()->data };

    /**
     * Update Resources [BindingUpdateInterval::ePerFrame]
     */
    const auto uniformEntry { data.at("RevMainSkyNode::UniformBuffer"sv) };
    auto& uniform { *_STD static_pointer_cast<Buffer, void>(uniformEntry) };

    const static math::mat4 clip_matrix = math::mat4(
        1.0F, 0.0F, 0.0F, 0.0F,
        0.0F, -1.0F, 0.0F, 0.0F,
        0.0F, 0.0F, 1.F, 0.F,
        0.0F, 0.0F, 0.F, 1.0F
    );

    const auto* camera { renderPass_->camera() };
    math::mat4 mv { camera->view() * math::mat4::make_identity() };
    mv[3] = math::vec4(0.F, 0.F, 0.F, 1.F);
    math::mat4 mvp { clip_matrix * camera->projection() * mv };
    uniform.write<math::mat4>(&mvp, 1ui32);

    /**
     * Prepare Command Buffer
     */
    const auto cmdEntry { data.at("RevMainSkyNode::CommandBuffer"sv) };
    auto& cmd { *_STD static_pointer_cast<CommandBuffer, void>(cmdEntry) };

    const auto entry { data.at("RevMainStage::Framebuffer"sv) };
    auto& frame { *_STD static_pointer_cast<Framebuffer, void>(entry) };

    // Warning: Experimental one time recording and reuse
    const auto checked { data.find("RevMainSkyNode::RecordedCommandBuffer"sv) };
    if (checked != data.end()) {
        return;
    }

    const vk::CommandBufferInheritanceInfo inheritance {
        _sharedNode->loRenderPass()->vkRenderPass(),
        0ui32,
        frame.vkFramebuffer(),
        VK_FALSE,
        {},
        {}
    };
    const vk::CommandBufferBeginInfo info {
        vk::CommandBufferUsageFlagBits::eRenderPassContinue | vk::CommandBufferUsageFlagBits::eSimultaneousUse,
        &inheritance
    };
    cmd.vkCommandBuffer().begin(info);

    /**
     *
     */
    cmd.bindPipeline(_pipeline.get(), {
        frame.width(),
        frame.height(),
        0.F,
        1.F
    });

    /**
     * Bind Shared Resources for the whole Frame
     */
    sptr<Vector<shader::DiscreteBindingGroup>> dbgs {
        _STD static_pointer_cast<Vector<shader::DiscreteBindingGroup>, void>(
            data.find("RevMainSkyNode::DiscreteBindingGroups"sv)->second
        )
    };

    for (u32 idx = 0; idx < dbgs->size(); ++idx) {

        const auto& grp { (*dbgs)[idx] };

        if (grp.super().interval() == shader::BindingUpdateInterval::ePerFrame) {
            cmd.bindDescriptor(idx, grp.vkSet());
        }

        if (grp.super().interval() == shader::BindingUpdateInterval::eMaterialUpdate) {
            cmd.bindDescriptor(idx, grp.vkSet());
        }
    }
    // TODO: _cmd->bindDescriptor({...});
}

void RevMainSkyNode::invoke(
    const non_owning_rptr<HORenderPass> renderPass_,
    const non_owning_rptr<RenderStagePass> stagePass_,
    const non_owning_rptr<SceneNodeModel> model_
) const {

    SCOPED_STOPWATCH

    const auto& data { renderPass_->state()->data };

    // Warning: Experimental one time recording and reuse
    const auto checked { data.find("RevMainSkyNode::RecordedCommandBuffer"sv) };
    if (checked != data.end()) {
        return;
    }

    /**
     * Get Command Buffer
     */
    const auto cmdEntry { data.at("RevMainSkyNode::CommandBuffer"sv) };
    auto& cmd { *_STD static_pointer_cast<CommandBuffer, void>(cmdEntry) };

    // TODO: cmd.bindDescriptor(...);
    cmd.draw(1, 0, 36, 0);
}

void RevMainSkyNode::after(
    const non_owning_rptr<HORenderPass> renderPass_,
    const non_owning_rptr<RenderStagePass> stagePass_
) const {

    SCOPED_STOPWATCH

    const auto& data { renderPass_->state()->data };

    // Warning: Experimental one time recording and reuse
    const auto checked {
        renderPass_->state()->data.insert_or_assign("RevMainSkyNode::RecordedCommandBuffer"sv, nullptr)
    };

    /**
     * Get Command Buffer
     */
    const auto cmdEntry { data.at("RevMainSkyNode::CommandBuffer"sv) };
    auto& cmd { *_STD static_pointer_cast<CommandBuffer, void>(cmdEntry) };

    // Stop recording
    if (checked.second) {
        /**
         * End Command Buffer
         */
        cmd.end();
    }

    // Commit Secondary to Primary
    {
        /**
         * Get Command Buffer
         */
        const auto primaryEntry { data.at("RevMainStage::CommandBuffer"sv) };
        auto& primary { *_STD static_pointer_cast<CommandBuffer, void>(primaryEntry) };

        /**
         * Submit
         */
        primary.vkCommandBuffer().executeCommands(1ui32, &cmd.vkCommandBuffer());
    }
}

void RevMainSkyNode::setupShader(cref<sptr<Device>> device_) {

    /**
     *
     */
    shader::Factory shaderFactory { device_ };
    ShaderStorage& shaderStorage { ShaderStorage::get() };

    /**
     * Prepare Prototype Bindings
     */
    shader::PrototypeBinding ubo {
        shader::BindingType::eUniformBuffer,
        1ui32,
        shader::BindingUpdateInterval::ePerFrame,
        "skyMainPassUbo"
    };

    shader::PrototypeBinding cubeMap {
        shader::BindingType::eImageSampler,
        2ui32,
        shader::BindingUpdateInterval::eMaterialUpdate,
        "skyMainPassCubeMap"
    };

    /**
     * Prepare Prototype Shader
     */
    shader::Prototype vertexPrototype { shader::ShaderType::eVertex, "skyMainPass" };

    auto vertexShaderCode { read_shader_file("resources/shader/mainpass_sky.vert.spv") };
    vertexPrototype.storeCode(vertexShaderCode.data(), vertexShaderCode.size());
    vertexPrototype.add(ubo);

    shader::Prototype fragmentPrototype { shader::ShaderType::eFragment, "skyMainPass" };

    auto fragmentShaderCode { read_shader_file("resources/shader/mainpass_sky.frag.spv") };
    fragmentPrototype.storeCode(fragmentShaderCode.data(), fragmentShaderCode.size());
    fragmentPrototype.add(cubeMap);

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
