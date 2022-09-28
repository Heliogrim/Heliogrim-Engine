#include "RevFinalComposeNode.hpp"

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

#include <Engine.GFX/VkFixedPipeline.hpp>
#include <Engine.GFX/API/VkTranslate.hpp>
#include <Engine.GFX/Buffer/Buffer.hpp>
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

#include "__macro.hpp"
#include "Engine.GFX/Graphics.hpp"
#include "Engine.GFX/Loader/RevTextureLoader.hpp"
#include "Engine.GFX/Texture/TextureFactory.hpp"

using namespace ember::engine::gfx::render;
using namespace ember::engine::gfx;
using namespace ember;

static Texture test {};

RevFinalComposeNode::RevFinalComposeNode() :
    RenderStageNode(),
    _device(nullptr) {}

RevFinalComposeNode::~RevFinalComposeNode() {
    RevFinalComposeNode::destroy();
}

void RevFinalComposeNode::setup(cref<sptr<Device>> device_) {

    // Store Device
    _device = device_;

    // LORenderPass
    setupLORenderPass();

    // Shader
    setupShader();

    // Pipeline
    setupPipeline();
}

void RevFinalComposeNode::destroy() {

    // Pipeline
    if (_pipeline) {
        _pipeline->destroy();
        _pipeline.reset();
    }

    // Shader
    for (auto& entry : _requiredDescriptorPools) {
        delete[] entry.pPoolSizes;

        entry.pPoolSizes = nullptr;
        entry.poolSizeCount = 0ui32;
    }

    for (auto& entry : _requiredBindingGroups) {
        _device->vkDevice().destroyDescriptorSetLayout(entry.vkSetLayout());
    }

    // LORenderPass
    destroyLORenderPass();
}

bool RevFinalComposeNode::allocate(const ptr<HORenderPass> renderPass_) {

    SCOPED_STOPWATCH

    const auto state { renderPass_->state() };

    /**
     * Allocate Command Buffer
     */
    auto cmd { _device->graphicsQueue()->pool()->make() };

    /**
     * Create Framebuffer
     */
    const auto* factory { TextureFactory::get() };

    Framebuffer framebuffer { _device };
    framebuffer.setExtent(renderPass_->target()->extent());
    framebuffer.setRenderPass(_loRenderPass);

    /**
     * Create Framebuffer :: Attach
     */
    auto target { renderPass_->target() };
    if (!target->vkView()) {
        factory->buildView(*target);
    }

    framebuffer.add({ target });

    /**
     *
     */
    framebuffer.setup();

    /**
     * Allocate Uniform Buffer
     */
    Buffer uniform {
        nullptr,
        nullptr,
        _device->vkDevice(),
        sizeof(math::vec3),
        vk::BufferUsageFlagBits::eUniformBuffer
    };

    const vk::BufferCreateInfo bci {
        vk::BufferCreateFlags {},
        uniform.size,
        uniform.usageFlags,
        vk::SharingMode::eExclusive,
        0,
        nullptr
    };

    uniform.buffer = _device->vkDevice().createBuffer(bci);
    assert(uniform.buffer);

    auto result {
        memory::allocate(&state->alloc, _device, uniform.buffer, MemoryProperty::eHostVisible, uniform.memory)
    };// TODO: Handle failed allocation
    uniform.bind();

    /**
     * Default insert data
     */
    math::vec3 pos {};
    uniform.write<math::vec3>(&pos, 1ui32);

    /**
     * Allocate Textures
     */
    if (!test) {
        RevTextureLoader loader { Session::get()->modules().graphics()->cacheCtrl() };
        test = loader.__tmp__load({ ""sv, R"(R:\\test.ktx)"sv });

        const vk::ImageMemoryBarrier imgBarrier {
            vk::AccessFlags {},
            vk::AccessFlagBits::eShaderRead,
            vk::ImageLayout::eTransferSrcOptimal,
            vk::ImageLayout::eShaderReadOnlyOptimal,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            test.buffer().image(),
            vk::ImageSubresourceRange {
                vk::ImageAspectFlagBits::eColor,
                0,
                test.mipLevels(),
                0,
                test.layer()
            }

        };

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
            1, &imgBarrier
        );

        iiCmd.end();
        iiCmd.submitWait();
        iiCmd.release();

        pool->lck().release();

        test.buffer()._vkLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        TextureFactory::get()->buildView(test);
    }

    const sptr<Framebuffer> depthFrame {
        _STD static_pointer_cast<Framebuffer, void>(state->data.find("RevDepthStage::Framebuffer"sv)->second)
    };
    const sptr<Texture> depth {
        depthFrame->attachments().at(0).unwrapped()
    };

    /**/
    const sptr<Texture> brdfLut {
        _STD static_pointer_cast<Texture, void>(state->data.find("RevEarlyBrdfLutNode::BrdfLut"sv)->second)
    };
    const sptr<Texture> prefiltered {
        _STD static_pointer_cast<Texture, void>(state->data.find("RevEarlyEnvPrefilterNode::PrefilterCube"sv)->second)
    };
    const sptr<Texture> irradiance {
        _STD static_pointer_cast<Texture, void>(state->data.find("RevEarlyEnvIrradianceNode::IrradianceCube"sv)->second)
    };
    /**/

    const sptr<Framebuffer> mainFrame {
        _STD static_pointer_cast<Framebuffer, void>(state->data.find("RevMainStage::Framebuffer"sv)->second)
    };
    const sptr<Texture> pbrAlbedo {
        mainFrame->attachments().at(0).unwrapped()
    };
    const sptr<Texture> pbrNormals {
        mainFrame->attachments().at(1).unwrapped()
    };
    const sptr<Texture> pbrPosition {
        mainFrame->attachments().at(2).unwrapped()
    };
    const sptr<Texture> pbrMrs {
        mainFrame->attachments().at(3).unwrapped()
    };

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
    dbgs[0].getById(shader::ShaderBinding::id_type { 2 }).storeAs(*pbrAlbedo, vk::ImageLayout::eShaderReadOnlyOptimal);
    dbgs[0].getById(shader::ShaderBinding::id_type { 3 }).storeAdv(*pbrNormals, vk::ImageLayout::eShaderReadOnlyOptimal,
        vk::SamplerAddressMode::eClampToBorder, vk::SamplerMipmapMode::eNearest, vk::Filter::eNearest);
    dbgs[0].getById(shader::ShaderBinding::id_type { 4 }).
            storeAs(*pbrPosition, vk::ImageLayout::eShaderReadOnlyOptimal);
    dbgs[0].getById(shader::ShaderBinding::id_type { 5 }).storeAs(*pbrMrs, vk::ImageLayout::eShaderReadOnlyOptimal);
    dbgs[0].getById(shader::ShaderBinding::id_type { 6 }).
            storeAs(*depth, vk::ImageLayout::eDepthStencilAttachmentOptimal);
    dbgs[0].getById(shader::ShaderBinding::id_type { 7 }).
            storeAdv(*brdfLut, vk::ImageLayout::eShaderReadOnlyOptimal, vk::SamplerAddressMode::eClampToEdge,
                vk::SamplerMipmapMode::eLinear, vk::Filter::eLinear, vk::BorderColor::eFloatOpaqueWhite);
    dbgs[0].getById(shader::ShaderBinding::id_type { 8 }).storeAs(*prefiltered, vk::ImageLayout::eShaderReadOnlyOptimal);
    dbgs[0].getById(shader::ShaderBinding::id_type { 9 }).storeAs(*irradiance, vk::ImageLayout::eShaderReadOnlyOptimal);

    /**
     * Store State
     */
    state->data.insert_or_assign("RevFinalComposeNode::CommandBuffer"sv,
        _STD make_shared<decltype(cmd)>(_STD move(cmd)));
    state->data.insert_or_assign("RevFinalComposeNode::Framebuffer"sv,
        _STD make_shared<decltype(framebuffer)>(_STD move(framebuffer)));
    state->data.insert_or_assign("RevFinalComposeNode::UniformBuffer"sv,
        _STD make_shared<decltype(uniform)>(_STD move(uniform)));
    state->data.insert_or_assign("RevFinalComposeNode::DiscreteBindingGroups"sv,
        _STD make_shared<decltype(dbgs)>(_STD move(dbgs)));
    state->data.insert_or_assign("RevFinalComposeNode::DescriptorPools"sv,
        _STD make_shared<decltype(pools)>(_STD move(pools)));

    return true;
}

bool RevFinalComposeNode::free(const ptr<HORenderPass> renderPass_) {

    SCOPED_STOPWATCH

    const auto state { renderPass_->state() };

    /**
     * Free Descriptors
     */
    auto it { state->data.find("RevFinalComposeNode::DiscreteBindingGroups"sv) };
    if (it != state->data.end()) {

        sptr<Vector<shader::DiscreteBindingGroup>> dbgs {
            _STD static_pointer_cast<Vector<shader::DiscreteBindingGroup>, void>(it->second)
        };

        const auto poolIt { state->data.find("RevFinalComposeNode::DescriptorPools"sv) };
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
            // const auto result { device->vkDevice().freeDescriptorSets(pool, 1ui32, &dbg.vkSet()) };
            // assert(result == vk::Result::eSuccess);
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
    it = state->data.find("RevFinalComposeNode::UniformBuffer"sv);
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
     *
     */
    it = state->data.find("RevFinalComposeNode::Framebuffer"sv);
    if (it != state->data.end()) {

        sptr<Framebuffer> buffer {
            _STD static_pointer_cast<Framebuffer, void>(it->second)
        };

        /**
         *
         */
        buffer->destroy();

        /*
        // Do not destroy attachments, cause target is most likely swapchain image and/or externally managed
        for (auto& entry : buffer->attachments()) {
            entry->destroy();
        }
         */

        /**
         *
         */
        state->data.erase(it);
    }

    /**
     * Free Command Buffers
     */
    it = state->data.find("RevFinalComposeNode::CommandBuffer"sv);
    if (it != state->data.end()) {

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
        state->data.erase(it);
    }

    return true;
}

Vector<RenderDataToken> RevFinalComposeNode::requiredToken() noexcept {
    return {};
}

Vector<RenderDataToken> RevFinalComposeNode::optionalToken() noexcept {
    return {};
}

void RevFinalComposeNode::before(
    const non_owning_rptr<HORenderPass> renderPass_,
    const non_owning_rptr<RenderStagePass> stagePass_
) const {

    SCOPED_STOPWATCH

    const auto& data { renderPass_->state()->data };

    /**
     * Prepare Command Buffer
     */
    const auto cmdEntry { data.at("RevFinalComposeNode::CommandBuffer"sv) };
    auto& cmd { *_STD static_pointer_cast<CommandBuffer, void>(cmdEntry) };
    cmd.begin();

    const auto entry { data.at("RevFinalComposeNode::Framebuffer"sv) };
    auto& framebuffer { *_STD static_pointer_cast<Framebuffer, void>(entry) };

    /**
     * Update Resources [BindingUpdateInterval::ePerFrame]
     */
    const auto uniformEntry { data.at("RevFinalComposeNode::UniformBuffer"sv) };
    auto& uniform { *_STD static_pointer_cast<Buffer, void>(uniformEntry) };

    const auto* camera { renderPass_->camera() };
    auto pos {camera->position()};
    uniform.write<math::vec3>(&pos, 1ui32);

    /**
     *
     */
    cmd.beginRenderPass(*_loRenderPass, framebuffer);
    cmd.bindPipeline(_pipeline.get(), {
        framebuffer.width(),
        framebuffer.height(),
        0.F,
        1.F
    });

    /**
     * Bind Shader Resources for the whole Frame
     */
    sptr<Vector<shader::DiscreteBindingGroup>> dbgs {
        _STD static_pointer_cast<Vector<shader::DiscreteBindingGroup>, void>(
            data.find("RevFinalComposeNode::DiscreteBindingGroups"sv)->second
        )
    };

    for (u32 idx = 0; idx < dbgs->size(); ++idx) {

        const auto& grp { (*dbgs)[idx] };

        if (grp.super().interval() == shader::BindingUpdateInterval::ePerFrame) {
            cmd.bindDescriptor(idx, grp.vkSet());
        }
    }
    // TODO: _cmd->bindDescriptor({...});
}

void RevFinalComposeNode::invoke(
    const non_owning_rptr<HORenderPass> renderPass_,
    const non_owning_rptr<RenderStagePass> stagePass_,
    const non_owning_rptr<SceneNodeModel> model_
) const {

    SCOPED_STOPWATCH

    const auto& data { renderPass_->state()->data };

    /**
     * Get Command Buffer
     */
    const auto cmdEntry { data.at("RevFinalComposeNode::CommandBuffer"sv) };
    auto& cmd { *_STD static_pointer_cast<CommandBuffer, void>(cmdEntry) };

    // TODO: cmd.bindDescriptor(...);
    cmd.draw(1, 0, 6, 0);
}

void RevFinalComposeNode::after(
    const non_owning_rptr<HORenderPass> renderPass_,
    const non_owning_rptr<RenderStagePass> stagePass_
) const {

    SCOPED_STOPWATCH

    /**
     * Get Command Buffer
     */
    const auto cmdEntry { renderPass_->state()->data.at("RevFinalComposeNode::CommandBuffer"sv) };
    auto& cmd { *_STD static_pointer_cast<CommandBuffer, void>(cmdEntry) };

    /**
     * End Command Buffer
     */
    cmd.endRenderPass();
    cmd.end();

    /**
     * Submit Command Buffer to CommandBatch
     */
    stagePass_->batch().push(cmd);
}

void RevFinalComposeNode::setupLORenderPass() {

    /**
     * LORenderPass
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

void RevFinalComposeNode::destroyLORenderPass() {

    if (_loRenderPass) {
        _loRenderPass->destroy();
        _loRenderPass.reset();
    }
}

void RevFinalComposeNode::setupShader() {

    /**
     *
     */
    shader::Factory shaderFactory { _device };
    ShaderStorage& shaderStorage { ShaderStorage::get() };

    /**
     * Prepare Prototype Bindings
     */
    shader::PrototypeBinding ubo {
        shader::BindingType::eUniformBuffer,
        1,
        shader::BindingUpdateInterval::ePerFrame,
        "finalPassUbo"
    };

    shader::PrototypeBinding albedo {
        shader::BindingType::eImageSampler,
        2,
        shader::BindingUpdateInterval::ePerFrame,
        "finalPassAlbedo"
    };

    shader::PrototypeBinding normal {
        shader::BindingType::eImageSampler,
        3,
        shader::BindingUpdateInterval::ePerFrame,
        "finalPassNormal"
    };

    shader::PrototypeBinding position {
        shader::BindingType::eImageSampler,
        4,
        shader::BindingUpdateInterval::ePerFrame,
        "finalPassPosition"
    };

    shader::PrototypeBinding mrs {
        shader::BindingType::eImageSampler,
        5,
        shader::BindingUpdateInterval::ePerFrame,
        "finalPassMrs"
    };

    shader::PrototypeBinding depth {
        shader::BindingType::eImageSampler,
        6,
        shader::BindingUpdateInterval::ePerFrame,
        "finalPassDepth"
    };

    shader::PrototypeBinding brdfLut {
        shader::BindingType::eImageSampler,
        7,
        shader::BindingUpdateInterval::ePerFrame,
        "brdfLut"
    };

    shader::PrototypeBinding prefilter {
        shader::BindingType::eImageSampler,
        8,
        shader::BindingUpdateInterval::ePerFrame,
        "prefilter"
    };

    shader::PrototypeBinding irradiance {
        shader::BindingType::eImageSampler,
        9,
        shader::BindingUpdateInterval::ePerFrame,
        "irradiance"
    };

    /**
     * Prepare Prototype Shader
     */
    shader::Prototype vertexPrototype { shader::ShaderType::eVertex, "finalPass" };

    auto vertexShaderCode { read_shader_file("resources/shader/finalpass.vert.spv") };
    vertexPrototype.storeCode(vertexShaderCode.data(), vertexShaderCode.size());
    vertexPrototype.add(ubo);

    shader::Prototype fragmentPrototype { shader::ShaderType::eFragment, "finalPass" };

    auto fragmentShaderCode { read_shader_file("resources/shader/finalpass.frag.spv") };
    fragmentPrototype.storeCode(fragmentShaderCode.data(), fragmentShaderCode.size());
    fragmentPrototype.add(ubo);
    fragmentPrototype.add(albedo);
    fragmentPrototype.add(normal);
    fragmentPrototype.add(position);
    fragmentPrototype.add(mrs);
    fragmentPrototype.add(depth);
    fragmentPrototype.add(brdfLut);
    fragmentPrototype.add(prefilter);
    fragmentPrototype.add(irradiance);

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
     * Acquire Descriptor Pools
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
            vk::DescriptorPoolCreateFlags {},
            1ui32,
            static_cast<u32>(sizes.size()),
            sizeMem
        };

        _requiredDescriptorPools.push_back(dpci);
        _requiredBindingGroups.push_back(group);
    }
}

void RevFinalComposeNode::setupPipeline() {

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

    _pipeline->vertexStage().shaderSlot().name() = "finalPass";
    _pipeline->fragmentStage().shaderSlot().name() = "finalPass";

    _pipeline->rasterizationStage().cullFace() = RasterCullFace::eNone;
    _pipeline->rasterizationStage().depthCheck() = false;

    static_cast<ptr<VkFixedPipeline>>(_pipeline.get())->blending().push_back(vk::PipelineColorBlendAttachmentState {
        VK_FALSE,
        vk::BlendFactor::eOne,
        vk::BlendFactor::eZero,
        vk::BlendOp::eAdd,
        vk::BlendFactor::eOne,
        vk::BlendFactor::eZero,
        vk::BlendOp::eAdd,
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
        vk::ColorComponentFlagBits::eA
    });

    /**
     *
     */
    _pipeline->setup();
}
