#include "RevDepthPassSkeletalStage.hpp"

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

#include "RevDepthPass.hpp"
#include "../../Graphics.hpp"
#include "../../VkFixedPipeline.hpp"
#include "../../API/VkTranslate.hpp"
#include "../../Memory/VkAllocator.hpp"
#include "../../Shader/Factory.hpp"
#include "../../Shader/ShaderStorage.hpp"
#include "../../Shader/DiscreteBinding.hpp"

using namespace ember::engine::gfx;
using namespace ember;

RevDepthPassSkeletalStage::RevDepthPassSkeletalStage(ptr<RevDepthPass> graphicPass_) :
    GraphicPassPipelineStage(),
    _graphicPass(graphicPass_) {}

void RevDepthPassSkeletalStage::setupShader() {

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
        "skeletalDepthPassUbo"
    };

    /**
     * Prepare Prototype Shader
     */
    shader::Prototype vertexPrototype { shader::ShaderType::eVertex, "skeletalDepthPass" };

    auto vertexShaderCode { read_shader_file(R"(resources/shader/depthpass_skeletal.vert.spv)") };
    vertexPrototype.storeCode(vertexShaderCode.data(), vertexShaderCode.size());
    vertexPrototype.add(ubo);

    shader::Prototype fragmentPrototype { shader::ShaderType::eFragment, "skeletalDepthPass" };

    auto fragmentShaderCode { read_shader_file(R"(resources/shader/depthpass_skeletal.frag.spv)") };
    fragmentPrototype.storeCode(fragmentShaderCode.data(), fragmentShaderCode.size());
    fragmentPrototype.add(ubo);

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
                sizes.push_back({ api::vkTranslateBindingType(binding.type()), 1ui32 });
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

void RevDepthPassSkeletalStage::setup() {

    SCOPED_STOPWATCH

    auto device = _graphicPass->device();

    /**
     * Command Buffer
     */
    _cmd = make_sptr<CommandBuffer>(_STD move(device->graphicsQueue()->pool()->make()));

    /**
     * Render Pass
     */
    _renderPass = make_sptr<pipeline::RenderPass>(device);
    assert(_renderPass);

    _renderPass->set(0, vk::AttachmentDescription {
        vk::AttachmentDescriptionFlags(),
        vk::Format::eD32SfloatS8Uint,
        vk::SampleCountFlagBits::e1,
        // Warning: Only first element at framebuffer have to clear attachment memory to reset
        vk::AttachmentLoadOp::eLoad,
        vk::AttachmentStoreOp::eStore,
        vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eDepthStencilAttachmentOptimal,
        vk::ImageLayout::eDepthStencilAttachmentOptimal
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

    _pipeline->vertexStage().shaderSlot().name() = "skeletalDepthPass";
    _pipeline->fragmentStage().shaderSlot().name() = "skeletalDepthPass";

    _pipeline->rasterizationStage().cullFace() = RasterCullFace::eBack;
    _pipeline->rasterizationStage().depthCheck() = true;
    _pipeline->rasterizationStage().depthCompare() = vk::CompareOp::eLessOrEqual;

    /**
     *
     */
    _pipeline->setup();
}

void RevDepthPassSkeletalStage::destroy() noexcept {

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
    if (_cmd && _cmd->vkCommandBuffer()) {
        _cmd->release();
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
        device->vkDevice().destroyDescriptorSetLayout(entry.vkSetLayout());
    }
}

void RevDepthPassSkeletalStage::allocateWith(const ptr<const RenderInvocation> invocation_,
    const ptr<RenderInvocationState> state_) {

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

    /**
     * Store State
     */
    state_->data.insert_or_assign("RevDepthPassSkeletalStage::UniformBuffer"sv,
        _STD make_shared<decltype(uniform)>(_STD move(uniform)));
    state_->data.insert_or_assign("RevDepthPassSkeletalStage::DiscreteBindingGroups"sv,
        _STD make_shared<decltype(dbgs)>(_STD move(dbgs)));
    state_->data.insert_or_assign("RevDepthPassSkeletalStage::DescriptorPools"sv,
        _STD make_shared<decltype(pools)>(_STD move(pools)));
}

void RevDepthPassSkeletalStage::freeWith(const ptr<const RenderInvocation> invocation_,
    const ptr<RenderInvocationState> state_) {

    /**
     *
     */
    const auto& device { _graphicPass->device() };

    /**
     * Free Descriptors
     */
    auto it { state_->data.find("RevDepthPassSkeletalStage::DiscreteBindingGroups"sv) };
    if (it != state_->data.end()) {

        sptr<Vector<shader::DiscreteBindingGroup>> dbgs {
            _STD static_pointer_cast<Vector<shader::DiscreteBindingGroup>, void>(it->second)
        };

        const auto poolIt { state_->data.find("RevDepthPassSkeletalStage::DescriptorPools"sv) };
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
    it = state_->data.find("RevDepthPassSkeletalStage::UniformBuffer"sv);
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

bool RevDepthPassSkeletalStage::check(ptr<const ProcessedModelBatch> batch_) noexcept {
    return batch_ != nullptr;
}

void RevDepthPassSkeletalStage::before(cref<GraphicPassStageContext> ctx_) {

    SCOPED_STOPWATCH

    /**
     * Prepare Command Buffer
     */
    _cmd->begin();

    const auto entry { ctx_.state.data.at("RevDepthPass::Framebuffer"sv) };
    auto& frame { *_STD static_pointer_cast<Framebuffer, void>(entry) };

    /**
     *
     */
    _cmd->beginRenderPass(*_renderPass, frame);
    _cmd->bindPipeline(_pipeline.get(), {
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
            ctx_.state.data.find("RevDepthPassSkeletalStage::DiscreteBindingGroups"sv)->second
        )
    };
    for (u32 idx = 0; idx < dbgs->size(); ++idx) {

        const auto& grp { (*dbgs)[idx] };

        if (grp.super().interval() == shader::BindingUpdateInterval::ePerFrame) {
            _cmd->bindDescriptor(idx, grp.vkSet());
        }
    }
    // TODO: _cmd->bindDescriptor({...});
    // TODO: Get uniform buffer object and use it for shared descriptor, cause depth pass only uses a ubo, no other resources required yet
}

void RevDepthPassSkeletalStage::process(cref<GraphicPassStageContext> ctx_, ptr<const ProcessedModelBatch> batch_) {

    SCOPED_STOPWATCH

    if (batch_->empty()) {
        return;
    }

    // TODO: cmd.bindDescriptor(...);
    _cmd->bindVertexBuffer(0, batch_->geometry().vertices, 0);
    // TODO: _cmd->bindVertexBuffer(1, batch_->geometry().instances, 0);
    _cmd->bindIndexBuffer(batch_->geometry().indices, 0);

    for (const auto& entry : batch_->executions()) {

        cref<DistinctBind> db = entry.bind;

        cref<DistinctGeometry> dg = entry.geometry;
        _cmd->drawIndexed(dg.instanceCount, dg.instanceOffset, dg.indexCount, dg.indexOffset, 0ui32);
    }
}

void RevDepthPassSkeletalStage::after(cref<GraphicPassStageContext> ctx_) {

    SCOPED_STOPWATCH

    /**
     * End Command Buffer
     */
    _cmd->endRenderPass();
    _cmd->end();

    /**
     * Submit Command Buffer to CommandBatch
     */
    ctx_.batch.push(*_cmd);
}

sptr<pipeline::RenderPass> RevDepthPassSkeletalStage::renderPass() const noexcept {
    return _renderPass;
}

sptr<FixedPipeline> RevDepthPassSkeletalStage::pipeline() const noexcept {
    return _pipeline;
}
