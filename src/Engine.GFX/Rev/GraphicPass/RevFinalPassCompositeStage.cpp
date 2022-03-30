#include "RevFinalPassCompositeStage.hpp"

#include <cassert>
#include <Engine.Common/Make.hpp>

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

#include "RevDepthPass.hpp"
#include "RevFinalPass.hpp"
#include "../../Graphics.hpp"
#include "../../VkFixedPipeline.hpp"
#include "../../API/VkTranslate.hpp"
#include "../../Memory/VkAllocator.hpp"
#include "../../Shader/Factory.hpp"
#include "../../Shader/ShaderStorage.hpp"
#include "../../Shader/DiscreteBinding.hpp"

#include "Engine.GFX/Texture/TextureFactory.hpp"
#include "Engine.GFX/Loader/RevTextureLoader.hpp"

using namespace ember::engine::gfx;
using namespace ember;

static Texture test {};

RevFinalPassCompositeStage::RevFinalPassCompositeStage(ptr<RevFinalPass> graphicPass_) :
    GraphicPassPipelineStage(),
    _graphicPass(graphicPass_) {}

void RevFinalPassCompositeStage::setupShader() {

    auto device = _graphicPass->device();

    /**
     *
     */
    shader::Factory shaderFactory { device };
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

void RevFinalPassCompositeStage::setup() {

    SCOPED_STOPWATCH

    auto device = _graphicPass->device();

    /**
     * Render Pass
     */
    _renderPass = make_sptr<pipeline::ApiRenderPass>(device);
    assert(_renderPass);

    _renderPass->set(0, vk::AttachmentDescription {
        vk::AttachmentDescriptionFlags(),
        /*api::vkTranslateFormat(swapchain->format()),*/
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
    _renderPass->setup();
    setupShader();

    /**
     * Fixed Pipeline
     */
    _pipeline = make_sptr<VkFixedPipeline>(device, _renderPass);
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

void RevFinalPassCompositeStage::destroy() noexcept {

    SCOPED_STOPWATCH

    const auto device { _graphicPass->device().get() };

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
    if (_renderPass) {
        _renderPass->destroy();
        _renderPass.reset();
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
    if (test) {
        test.destroy();
    }

    /**
     *
     */
    for (auto& entry : _requiredBindingGroups) {
        device->vkDevice().destroyDescriptorSetLayout(entry.vkSetLayout());
    }
}

void RevFinalPassCompositeStage::allocateWith(const ptr<const RenderPass> invocation_,
    const ptr<RenderPassState> state_) {

    /**
     *
     */
    const auto& device { _graphicPass->device() };

    /**
     * Allocate Command Buffer
     */
    auto cmd { device->graphicsQueue()->pool()->make() };

    /**
     * Allocate Uniform Buffer
     */
    Buffer uniform {
        nullptr,
        nullptr,
        device->vkDevice(),
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

    uniform.buffer = device->vkDevice().createBuffer(bci);
    assert(uniform.buffer);

    ptr<VkAllocator> alloc {
        VkAllocator::makeForBuffer(device, uniform.buffer, vk::MemoryPropertyFlagBits::eHostVisible)
    };

    const vk::MemoryRequirements req { device->vkDevice().getBufferMemoryRequirements(uniform.buffer) };
    uniform.memory = alloc->allocate(req.size);
    uniform.bind();

    delete alloc;

    /**
     * Default insert data
     */
    math::vec3 pos {};
    uniform.write<math::vec3>(&pos, 1ui32);

    /**
     * Allocate Textures
     */
    if (!test) {
        RevTextureLoader loader { _graphicPass->device() };
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

        auto pool = device->graphicsQueue()->pool();
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
        _STD static_pointer_cast<Framebuffer, void>(state_->data.find("RevDepthPass::Framebuffer"sv)->second)
    };
    const sptr<Texture> depth {
        depthFrame->attachments().at(0).unwrapped()
    };

    const sptr<Framebuffer> mainFrame {
        _STD static_pointer_cast<Framebuffer, void>(state_->data.find("RevMainPass::Framebuffer"sv)->second)
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

        vk::DescriptorPool pool { device->vkDevice().createDescriptorPool(_requiredDescriptorPools[rdp]) };
        assert(pool);

        const auto& grp { _requiredBindingGroups[rdp] };

        vk::DescriptorSet set { device->vkDevice().allocateDescriptorSets({ pool, 1ui32, &grp.vkSetLayout() })[0] };
        assert(set);

        dbgs.push_back(grp.useDiscrete(set));
        pools.push_back(pool);
    }

    /**
     * Pre Store
     */
    dbgs[0].getById(shader::ShaderBinding::id_type { 1 }).store(uniform);
    dbgs[0].getById(shader::ShaderBinding::id_type { 2 }).storeAs(*pbrAlbedo, vk::ImageLayout::eShaderReadOnlyOptimal);
    dbgs[0].getById(shader::ShaderBinding::id_type { 3 }).storeAs(*pbrNormals, vk::ImageLayout::eShaderReadOnlyOptimal);
    dbgs[0].getById(shader::ShaderBinding::id_type { 4 }).
            storeAs(*pbrPosition, vk::ImageLayout::eShaderReadOnlyOptimal);
    dbgs[0].getById(shader::ShaderBinding::id_type { 5 }).storeAs(*pbrMrs, vk::ImageLayout::eShaderReadOnlyOptimal);
    dbgs[0].getById(shader::ShaderBinding::id_type { 6 }).
            storeAs(*depth, vk::ImageLayout::eDepthStencilAttachmentOptimal);

    /**
     * Store State
     */
    state_->data.insert_or_assign("RevFinalPassCompositeStage::CommandBuffer"sv,
        _STD make_shared<decltype(cmd)>(_STD move(cmd)));
    state_->data.insert_or_assign("RevFinalPassCompositeStage::UniformBuffer"sv,
        _STD make_shared<decltype(uniform)>(_STD move(uniform)));
    state_->data.insert_or_assign("RevFinalPassCompositeStage::DiscreteBindingGroups"sv,
        _STD make_shared<decltype(dbgs)>(_STD move(dbgs)));
    state_->data.insert_or_assign("RevFinalPassCompositeStage::DescriptorPools"sv,
        _STD make_shared<decltype(pools)>(_STD move(pools)));
}

void RevFinalPassCompositeStage::freeWith(const ptr<const RenderPass> invocation_,
    const ptr<RenderPassState> state_) {

    /**
     *
     */
    const auto& device { _graphicPass->device() };

    /**
     * Free Descriptors
     */
    auto it { state_->data.find("RevFinalPassCompositeStage::DiscreteBindingGroups"sv) };
    if (it != state_->data.end()) {

        sptr<Vector<shader::DiscreteBindingGroup>> dbgs {
            _STD static_pointer_cast<Vector<shader::DiscreteBindingGroup>, void>(it->second)
        };

        const auto poolIt { state_->data.find("RevFinalPassCompositeStage::DescriptorPools"sv) };
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
            device->vkDevice().freeDescriptorSets(pool, 1ui32, &dbg.vkSet())
            #endif
        }

        /**
         *
         */
        for (const auto& entry : *pools) {
            device->vkDevice().destroyDescriptorPool(entry);
        }

        /**
         *
         */
        state_->data.erase(it);
        state_->data.erase(poolIt);
    }

    /**
     * Free Buffers
     */
    it = state_->data.find("RevFinalPassCompositeStage::UniformBuffer"sv);
    if (it != state_->data.end()) {

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
        state_->data.erase(it);
    }

    /**
     * Free Command Buffers
     */
    it = state_->data.find("RevFinalPassCompositeStage::CommandBuffer"sv);
    if (it != state_->data.end()) {

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
        state_->data.erase(it);
    }
}

bool RevFinalPassCompositeStage::check(ptr<const ProcessedModelBatch> batch_) noexcept {
    return batch_ == nullptr;
}

void RevFinalPassCompositeStage::before(const ptr<const RenderContext> ctx_, cref<GraphicPassStageContext> stageCtx_) {

    SCOPED_STOPWATCH

    const auto& data { ctx_->state()->data };

    /**
     * Prepare Command Buffer
     */
    const auto cmdEntry { data.at("RevFinalPassCompositeStage::CommandBuffer"sv) };
    auto& cmd { *_STD static_pointer_cast<CommandBuffer, void>(cmdEntry) };
    cmd.begin();

    const auto entry { data.at("RevFinalPass::Framebuffer"sv) };
    auto& frame { *_STD static_pointer_cast<Framebuffer, void>(entry) };

    /**
     * Update Resources [BindingUpdateInterval::ePerFrame]
     */
    const auto uniformEntry { data.at("RevFinalPassCompositeStage::UniformBuffer"sv) };
    auto& uniform { *_STD static_pointer_cast<Buffer, void>(uniformEntry) };

    const auto* camera { ctx_->camera() };
    uniform.write<math::vec3>(&camera->position(), 1ui32);

    /**
     *
     */
    cmd.beginRenderPass(*_renderPass, frame);
    cmd.bindPipeline(_pipeline.get(), {
        frame.width(),
        frame.height(),
        0.F,
        1.F
    });

    /**
     * Bind Shader Resources for the whole Frame
     */
    sptr<Vector<shader::DiscreteBindingGroup>> dbgs {
        _STD static_pointer_cast<Vector<shader::DiscreteBindingGroup>, void>(
            data.find("RevFinalPassCompositeStage::DiscreteBindingGroups"sv)->second
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

void RevFinalPassCompositeStage::process(const ptr<const RenderContext> ctx_, cref<GraphicPassStageContext> stageCtx_,
    ptr<const ProcessedModelBatch> batch_) {

    SCOPED_STOPWATCH

    const auto& data { ctx_->state()->data };

    /**
     * Get Command Buffer
     */
    const auto cmdEntry { data.at("RevFinalPassCompositeStage::CommandBuffer"sv) };
    auto& cmd { *_STD static_pointer_cast<CommandBuffer, void>(cmdEntry) };

    // TODO: cmd.bindDescriptor(...);
    cmd.draw(1, 0, 6, 0);
}

void RevFinalPassCompositeStage::after(const ptr<const RenderContext> ctx_, cref<GraphicPassStageContext> stageCtx_) {

    SCOPED_STOPWATCH

    /**
     * Get Command Buffer
     */
    const auto cmdEntry { ctx_->state()->data.at("RevFinalPassCompositeStage::CommandBuffer"sv) };
    auto& cmd { *_STD static_pointer_cast<CommandBuffer, void>(cmdEntry) };

    /**
     * End Command Buffer
     */
    cmd.endRenderPass();
    cmd.end();

    /**
     * Submit Command Buffer to CommandBatch
     */
    stageCtx_.batch.push(cmd);
}

sptr<pipeline::ApiRenderPass> RevFinalPassCompositeStage::renderPass() const noexcept {
    return _renderPass;
}

sptr<FixedPipeline> RevFinalPassCompositeStage::pipeline() const noexcept {
    return _pipeline;
}
