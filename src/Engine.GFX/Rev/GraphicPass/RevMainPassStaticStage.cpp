#include "RevMainPassStaticStage.hpp"

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

#include "RevMainPass.hpp"
#include "__macro.hpp"
#include "../../Graphics.hpp"
#include "../../VkFixedPipeline.hpp"
#include "../../API/VkTranslate.hpp"
#include "../../Memory/VkAllocator.hpp"
#include "../../Shader/Factory.hpp"
#include "../../Shader/ShaderStorage.hpp"
#include "Engine.GFX/Loader/RevTextureLoader.hpp"
#include "Engine.GFX/Texture/TextureFactory.hpp"

using namespace ember::engine::gfx;
using namespace ember;

static Texture testAlbedo {};
static Texture testNormal {};
static Texture testRoughness {};
static Texture testSpecular {};

RevMainPassStaticStage::RevMainPassStaticStage(ptr<RevMainPass> graphicPass_) :
    GraphicPassPipelineStage(),
    _graphicPass(graphicPass_) {}

void RevMainPassStaticStage::setupShader() {

    auto device { _graphicPass->device() };

    /**
     *
     */
    shader::Factory shaderFactory { device };
    ShaderStorage& shaderStorage { ShaderStorage::get() };

    /**
     * Prepare Protoype Bindings
     */
    shader::PrototypeBinding ubo {
        shader::BindingType::eUniformBuffer,
        1ui32,
        shader::BindingUpdateInterval::ePerFrame,
        "staticMainPassUbo"
    };
    shader::PrototypeBinding mubo {
        shader::BindingType::eStorageBuffer,
        2ui32,
        shader::BindingUpdateInterval::ePerFrame,
        "staticMainPassModel"
    };

    shader::PrototypeBinding albedo {
        shader::BindingType::eImageSampler,
        3ui32,
        shader::BindingUpdateInterval::eMaterialUpdate,
        "staticMainPassAlbedo"
    };

    shader::PrototypeBinding normal {
        shader::BindingType::eImageSampler,
        4ui32,
        shader::BindingUpdateInterval::eMaterialUpdate,
        "staticMainPassNormal"
    };

    shader::PrototypeBinding roughness {
        shader::BindingType::eImageSampler,
        5ui32,
        shader::BindingUpdateInterval::eMaterialUpdate,
        "staticMainPassRoughness"
    };

    shader::PrototypeBinding specular {
        shader::BindingType::eImageSampler,
        6ui32,
        shader::BindingUpdateInterval::eMaterialUpdate,
        "staticMainPassSpecular"
    };

    /**
     * Prepare Prototype Shader
     */
    shader::Prototype vertexPrototype { shader::ShaderType::eVertex, "staticMainPass" };

    auto vertexShaderCode { read_shader_file("resources/shader/mainpass_static.vert.spv") };
    vertexPrototype.storeCode(vertexShaderCode.data(), vertexShaderCode.size());
    vertexPrototype.add(ubo);
    vertexPrototype.add(mubo);

    shader::Prototype fragmentPrototype { shader::ShaderType::eFragment, "staticMainPass" };
    auto fragmentShaderCode { read_shader_file("resources/shader/mainpass_static.frag.spv") };
    fragmentPrototype.storeCode(fragmentShaderCode.data(), fragmentShaderCode.size());
    fragmentPrototype.add(albedo);
    fragmentPrototype.add(normal);
    fragmentPrototype.add(roughness);
    fragmentPrototype.add(specular);

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

void RevMainPassStaticStage::setup() {

    SCOPED_STOPWATCH

    auto device = _graphicPass->device();

    /**
     *
     */
    setupShader();

    /**
     * Fixed Pipeline
     */
    _pipeline = make_sptr<VkFixedPipeline>(device, _graphicPass->renderPass());
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

    _pipeline->vertexStage().shaderSlot().name() = "staticMainPass";
    _pipeline->fragmentStage().shaderSlot().name() = "staticMainPass";

    _pipeline->rasterizationStage().cullFace() = RasterCullFace::eBack;
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
    if (!testAlbedo) {
        RevTextureLoader loader { _graphicPass->device() };
        testAlbedo = loader.__tmp__load({ ""sv, R"(R:\\albedo.ktx)" });
        testNormal = loader.__tmp__load({ ""sv, R"(R:\\normal.ktx)" });
        testRoughness = loader.__tmp__load({ ""sv, R"(R:\\roughness.ktx)" });
        testSpecular = loader.__tmp__load({ ""sv, R"(R:\\specular.ktx)" });

        Vector<vk::ImageMemoryBarrier> imgBarriers {};
        imgBarriers.push_back({
            vk::AccessFlags {},
            vk::AccessFlagBits::eShaderRead,
            vk::ImageLayout::eTransferSrcOptimal,
            vk::ImageLayout::eShaderReadOnlyOptimal,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            testAlbedo.buffer().image(),
            vk::ImageSubresourceRange {
                vk::ImageAspectFlagBits::eColor,
                0,
                testAlbedo.mipLevels(),
                0,
                testAlbedo.layer()
            }
        });

        imgBarriers.push_back({
            vk::AccessFlags {},
            vk::AccessFlagBits::eShaderRead,
            vk::ImageLayout::eTransferSrcOptimal,
            vk::ImageLayout::eShaderReadOnlyOptimal,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            testNormal.buffer().image(),
            vk::ImageSubresourceRange {
                vk::ImageAspectFlagBits::eColor,
                0,
                testNormal.mipLevels(),
                0,
                testNormal.layer()
            }
        });

        imgBarriers.push_back({
            vk::AccessFlags {},
            vk::AccessFlagBits::eShaderRead,
            vk::ImageLayout::eTransferSrcOptimal,
            vk::ImageLayout::eShaderReadOnlyOptimal,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            testRoughness.buffer().image(),
            vk::ImageSubresourceRange {
                vk::ImageAspectFlagBits::eColor,
                0,
                testRoughness.mipLevels(),
                0,
                testRoughness.layer()
            }
        });

        imgBarriers.push_back({
            vk::AccessFlags {},
            vk::AccessFlagBits::eShaderRead,
            vk::ImageLayout::eTransferSrcOptimal,
            vk::ImageLayout::eShaderReadOnlyOptimal,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            testSpecular.buffer().image(),
            vk::ImageSubresourceRange {
                vk::ImageAspectFlagBits::eColor,
                0,
                testSpecular.mipLevels(),
                0,
                testSpecular.layer()
            }
        });

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
            static_cast<uint32_t>(imgBarriers.size()), imgBarriers.data()
        );

        iiCmd.end();
        iiCmd.submitWait();
        iiCmd.release();

        pool->lck().release();

        testAlbedo.buffer()._vkLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        testNormal.buffer()._vkLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        testRoughness.buffer()._vkLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        testSpecular.buffer()._vkLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

        TextureFactory::get()->buildView(testAlbedo);
        TextureFactory::get()->buildView(testNormal);
        TextureFactory::get()->buildView(testRoughness);
        TextureFactory::get()->buildView(testSpecular);
    }
}

void RevMainPassStaticStage::destroy() noexcept {

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
    for (auto& entry : _requiredDescriptorPools) {
        delete[] entry.pPoolSizes;

        entry.pPoolSizes = nullptr;
        entry.poolSizeCount = 0ui32;
    }

    // TODO:
    if (testAlbedo) {
        testAlbedo.destroy();
    }
    if (testNormal) {
        testNormal.destroy();
    }
    if (testRoughness) {
        testRoughness.destroy();
    }
    if (testSpecular) {
        testSpecular.destroy();
    }

    /**
     *
     */
    for (auto& entry : _requiredBindingGroups) {
        device->vkDevice().destroyDescriptorSetLayout(entry.vkSetLayout());
    }
}

void RevMainPassStaticStage::allocateWith(const ptr<const RenderPass> invocation_,
    const ptr<RenderPassState> state_) {

    /**
     *
     */
    const auto& device { _graphicPass->device() };

    /**
     * Allocate Uniform Buffer
     */
    Buffer uniform {
        nullptr,
        nullptr,
        device->vkDevice(),
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
    const ptr<Camera> camera { invocation_->camera() };
    math::mat4 mvp { camera->projection() * camera->view() * math::mat4::make_identity() };
    uniform.write<math::mat4>(&mvp, 1ui32);

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
    dbgs[1].getById(shader::ShaderBinding::id_type { 3 }).store(testAlbedo);
    dbgs[1].getById(shader::ShaderBinding::id_type { 4 }).store(testNormal);
    dbgs[1].getById(shader::ShaderBinding::id_type { 5 }).store(testRoughness);
    dbgs[1].getById(shader::ShaderBinding::id_type { 6 }).store(testSpecular);

    /**
     * Store State
     */
    state_->data.insert_or_assign("RevMainPassStaticStage::UniformBuffer"sv,
        _STD make_shared<decltype(uniform)>(_STD move(uniform)));
    state_->data.insert_or_assign("RevMainPassStaticStage::DiscreteBindingGroups"sv,
        _STD make_shared<decltype(dbgs)>(_STD move(dbgs)));
    state_->data.insert_or_assign("RevMainPassStaticStage::DescriptorPools"sv,
        _STD make_shared<decltype(pools)>(_STD move(pools)));
}

void RevMainPassStaticStage::freeWith(const ptr<const RenderPass> invocation_,
    const ptr<RenderPassState> state_) {

    /**
     *
     */
    const auto& device { _graphicPass->device() };

    /**
     * Free Descriptors
     */
    auto it { state_->data.find("RevMainPassStaticStage::DiscreteBindingGroups"sv) };
    if (it != state_->data.end()) {

        sptr<Vector<shader::DiscreteBindingGroup>> dbgs {
            _STD static_pointer_cast<Vector<shader::DiscreteBindingGroup>, void>(it->second)
        };

        const auto poolIt { state_->data.find("RevMainPassStaticStage::DescriptorPools"sv) };
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
    it = state_->data.find("RevMainPassStaticStage::UniformBuffer"sv);
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
}

bool RevMainPassStaticStage::check(ptr<const ProcessedModelBatch> batch_) noexcept {
    return batch_ != nullptr;
}

void RevMainPassStaticStage::before(const ptr<const RenderContext> ctx_, cref<GraphicPassStageContext> stageCtx_) {

    SCOPED_STOPWATCH

    const auto& data { ctx_->state()->data };

    sptr<Vector<shader::DiscreteBindingGroup>> dbgs {
        _STD static_pointer_cast<Vector<shader::DiscreteBindingGroup>, void>(
            data.find("RevMainPassStaticStage::DiscreteBindingGroups"sv)->second
        )
    };

    /**
     * Prepare Command Buffer
     */
    const auto cmdEntry { data.at("RevMainPass::CommandBuffer"sv) };
    auto& cmd { *_STD static_pointer_cast<CommandBuffer, void>(cmdEntry) };

    const auto entry { ctx_->state()->data.at("RevMainPass::Framebuffer"sv) };
    auto& frame { *_STD static_pointer_cast<Framebuffer, void>(entry) };

    /**
     * Update Resources [BindingUpdateInterval::ePerFrame]
     */
    const auto uniformEntry { data.at("RevMainPassStaticStage::UniformBuffer"sv) };
    auto& uniform { *_STD static_pointer_cast<Buffer, void>(uniformEntry) };

    const static math::mat4 clip_matrix = math::mat4(
        1.0F, 0.0F, 0.0F, 0.0F,
        0.0F, -1.0F, 0.0F, 0.0F,
        0.0F, 0.0F, 0.5F, 0.5F,
        0.0F, 0.0F, 0.5F, 1.0F
    );

    const auto* camera { ctx_->camera() };
    math::mat4 mvpc { clip_matrix * camera->projection() * camera->view() * math::mat4::make_identity() };
    uniform.write<math::mat4>(&mvpc, 1ui32);

    // Warning: Temporary
    {
        sptr<Buffer> modelUniform {
            _STD static_pointer_cast<Buffer, void>(
                ctx_->state()->data.at("RevMainPassModelProcessor::InstanceBuffer"sv)
            )
        };

        dbgs->front().getById(2).store(*modelUniform);
    }

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

void RevMainPassStaticStage::process(const ptr<const RenderContext> ctx_, cref<GraphicPassStageContext> stageCtx_,
    ptr<const ProcessedModelBatch> batch_) {

    SCOPED_STOPWATCH

    if (batch_->empty()) {
        return;
    }

    const auto& data { ctx_->state()->data };

    /**
     * Get Command Buffer
     */
    const auto cmdEntry { data.at("RevMainPass::CommandBuffer"sv) };
    auto& cmd { *_STD static_pointer_cast<CommandBuffer, void>(cmdEntry) };

    // TODO: cmd.bindDescriptor(...);
    cmd.bindVertexBuffer(0, batch_->geometry().vertices, 0);
    cmd.bindIndexBuffer(batch_->geometry().indices, 0);

    for (const auto& entry : batch_->executions()) {

        cref<DistinctBind> db = entry.bind;
        cref<DistinctGeometry> dg = entry.geometry;

        /**
         *
         */
        // TODO: Descriptor Bindings

        /**
         * Draw Call
         */
        cmd.drawIndexed(dg.instanceCount, dg.instanceOffset, dg.indexCount, dg.indexOffset, 0ui32);
    }
}

void RevMainPassStaticStage::after(const ptr<const RenderContext> ctx_, cref<GraphicPassStageContext> stageCtx_) {

    SCOPED_STOPWATCH

    #if FALSE
    const auto& data { ctx_->state()->data };

    /**
     * Get Command Buffer
     */
    const auto cmdEntry { data.at("RevMainPass::CommandBuffer"sv) };
    auto& cmd { *_STD static_pointer_cast<CommandBuffer, void>(cmdEntry) };
    #endif
}

sptr<FixedPipeline> RevMainPassStaticStage::pipeline() const noexcept {
    return _pipeline;
}
