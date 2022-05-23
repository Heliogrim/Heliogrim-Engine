#include "RevDepthStaticNode.hpp"

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
#include <Engine.GFX/Renderer/RenderPassState.hpp>
#include <Engine.GFX/Renderer/RenderStagePass.hpp>
#include <Engine.GFX/Shader/DiscreteBindingGroup.hpp>
#include <Engine.GFX/Shader/Factory.hpp>
#include <Engine.GFX/Shader/Prototype.hpp>
#include <Engine.GFX/Shader/PrototypeBinding.hpp>
#include <Engine.GFX/Shader/ShaderStorage.hpp>

#include "__macro.hpp"
#include "RevDepthSharedNode.hpp"
#include "Ember/StaticGeometryComponent.hpp"
#include "Engine.Common/Math/Coordinates.hpp"
#include "Engine.GFX/Resource/StaticGeometryResource.hpp"
#include "Engine.GFX/Scene/StaticGeometryModel.hpp"
#include "Engine.Reflect/EmberReflect.hpp"
#include <Engine.GFX/Scene/StaticGeometryBatch.hpp>

using namespace ember::engine::gfx::render;
using namespace ember;

RevDepthStaticNode::RevDepthStaticNode(const ptr<RevDepthSharedNode> sharedNode_) :
    RenderStageNode(),
    _modelTypes({
        EmberClass::stid<StaticGeometryModel>()
    }),
    _sharedNode(sharedNode_) {}

void RevDepthStaticNode::setup(cref<sptr<Device>> device_) {

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

    _pipeline->vertexStage().shaderSlot().name() = "staticDepthPass";
    _pipeline->fragmentStage().shaderSlot().name() = "staticDepthPass";

    _pipeline->rasterizationStage().cullFace() = RasterCullFace::eBack;
    _pipeline->rasterizationStage().depthCompare() = vk::CompareOp::eLess;

    /**
     * Attention: Need to guarantee invariant from gl_Position calculation
     *  If not guaranteed, z-fighting will occur
     *
     * @see https://stackoverflow.com/a/46920273
     */
    // Prevent depth artifacts while re-using depth buffer
    //_pipeline->rasterizationStage().setDepthBias(1.25F, 0.F, 1.75F);
    //_pipeline->rasterizationStage().setDepthBias(0.F, 0.F, 1.F);

    /**
     *
     */
    _pipeline->setup();
}

void RevDepthStaticNode::destroy() {

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

}

bool RevDepthStaticNode::allocate(const ptr<HORenderPass> renderPass_) {

    SCOPED_STOPWATCH

    const auto state { renderPass_->state() };

    /**
     * Allocate Command Buffer
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
        0,
        nullptr
    };

    uniform.buffer = _device->vkDevice().createBuffer(bci);
    assert(uniform.buffer);

    auto result {
        memory::allocate(&state->alloc, _device, uniform.buffer, MemoryProperty::eHostVisible, uniform.memory)
    };// TODO: Should assert or handle failed memory allocation
    uniform.bind();

    /**
     * Default insert data
     */
    const ptr<Camera> camera { renderPass_->camera() };
    math::mat4 mvp { camera->projection() * camera->view() * math::mat4::make_identity() };
    uniform.write<math::mat4>(&mvp, 1ui32);

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

    /**
     * Store State
     */
    state->data.insert_or_assign("RevDepthStaticNode::CommandBuffer"sv,
        _STD make_shared<decltype(cmd)>(_STD move(cmd)));
    state->data.insert_or_assign("RevDepthStaticNode::UniformBuffer"sv,
        _STD make_shared<decltype(uniform)>(_STD move(uniform)));
    state->data.insert_or_assign("RevDepthStaticNode::DiscreteBindingGroups"sv,
        _STD make_shared<decltype(dbgs)>(_STD move(dbgs)));
    state->data.insert_or_assign("RevDepthStaticNode::DescriptorPools"sv,
        _STD make_shared<decltype(pools)>(_STD move(pools)));

    return true;
}

bool RevDepthStaticNode::free(const ptr<HORenderPass> renderPass_) {

    SCOPED_STOPWATCH

    const auto state { renderPass_->state() };

    /**
     * Free Descriptors
     */
    auto it { state->data.find("RevDepthStaticNode::DiscreteBindingGroups"sv) };
    if (it != state->data.end()) {

        sptr<Vector<shader::DiscreteBindingGroup>> dbgs {
            _STD static_pointer_cast<Vector<shader::DiscreteBindingGroup>, void>(it->second)
        };

        const auto poolIt { state->data.find("RevDepthStaticNode::DescriptorPools"sv) };
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
    it = state->data.find("RevDepthStaticNode::UniformBuffer"sv);
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
    it = renderPass_->state()->data.find("RevDepthStaticNode::CommandBuffer"sv);
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

const non_owning_rptr<const Vector<type_id>> RevDepthStaticNode::modelTypes() const noexcept {
    return &_modelTypes;
}

void RevDepthStaticNode::before(
    const non_owning_rptr<HORenderPass> renderPass_,
    const non_owning_rptr<RenderStagePass> stagePass_
) const {

    SCOPED_STOPWATCH

    const auto& data { renderPass_->state()->data };

    /**
     * Prepare Command Buffer
     */
    const auto cmdEntry { data.at("RevDepthStaticNode::CommandBuffer"sv) };
    auto& cmd { *_STD static_pointer_cast<CommandBuffer, void>(cmdEntry) };

    const auto entry { data.at("RevDepthStage::Framebuffer"sv) };
    auto& framebuffer { *_STD static_pointer_cast<Framebuffer, void>(entry) };

    const vk::CommandBufferInheritanceInfo inheritance {
        _sharedNode->loRenderPass()->vkRenderPass(),
        0ui32,
        framebuffer.vkFramebuffer(),
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
        framebuffer.width(),
        framebuffer.height(),
        0.F,
        1.F
    });

    /**
     * Update Resources [BindingUpdateInterval::ePerFrame]
     */
    const auto uniformEntry { data.at("RevDepthStaticNode::UniformBuffer"sv) };
    auto& uniform { *_STD static_pointer_cast<Buffer, void>(uniformEntry) };

    const static math::mat4 clip_matrix = math::mat4(
        1.0F, 0.0F, 0.0F, 0.0F,
        0.0F, -1.0F, 0.0F, 0.0F,
        0.0F, 0.0F, 0.5F, 0.5F,
        0.0F, 0.0F, 0.5F, 1.0F
    );

    const auto* camera { renderPass_->camera() };
    math::mat4 mvpc { clip_matrix * camera->projection() * camera->view() * math::mat4::make_identity() };
    uniform.write<math::mat4>(&mvpc, 1ui32);

    /**
     * Bind Shared Resources for the whole Frame
     */
    sptr<Vector<shader::DiscreteBindingGroup>> dbgs {
        _STD static_pointer_cast<Vector<shader::DiscreteBindingGroup>, void>(
            data.find("RevDepthStaticNode::DiscreteBindingGroups"sv)->second
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
}

void RevDepthStaticNode::invoke(
    const non_owning_rptr<HORenderPass> renderPass_,
    const non_owning_rptr<RenderStagePass> stagePass_,
    const non_owning_rptr<SceneNodeModel> model_
) const {

    SCOPED_STOPWATCH

    auto* state { renderPass_->state().get() };
    const auto& data { renderPass_->state()->data };

    /**
     * Get Command Buffer
     */
    const auto cmdEntry { data.at("RevDepthStaticNode::CommandBuffer"sv) };
    auto& cmd { *_STD static_pointer_cast<CommandBuffer, void>(cmdEntry) };

    // Temporary
    const auto* owner { static_cast<const ptr<StaticGeometryComponent>>(model_->owner()) };
    const auto* model { static_cast<const ptr<StaticGeometryModel>>(model_) };

    if (!model->geometryResource()->isLoaded()) {
        return;
    }

    const auto* sgr { static_cast<const ptr<StaticGeometryResource>>(model->geometryResource()) };

    /**
     *
     */
    cmd.bindVertexBuffer(0, sgr->_vertexData.buffer, 0);
    cmd.bindIndexBuffer(sgr->_indexData.buffer, 0);

    u32 sbdIdx { 0ui32 };
    ptr<const shader::ShaderBindingGroup> sbg { nullptr };
    for (; sbdIdx < _requiredBindingGroups.size(); ++sbdIdx) {
        const auto& entry { _requiredBindingGroups[sbdIdx] };
        if (entry.interval() == shader::BindingUpdateInterval::ePerInstance) {
            sbg = &entry;
            break;
        }
    }

    if (sbg == nullptr) {
        return;
    }

    auto* const batch { model_->batch(state) };
    auto* const casted { static_cast<const ptr<StaticGeometryBatch>>(batch) };

    if (casted->cdb != nullptr) {
        auto& dbg { casted->cdb->binding() };
        cmd.bindDescriptor(sbdIdx, dbg.vkSet());

    } else {
        /**
         * Acquire Descriptor for model data, push data and bind it
         */
        auto& cache { renderPass_->state()->bindingCache };
        auto dbg { cache.allocate(*sbg) };

        dbg.getById(2ui32).store(casted->instance);
        cmd.bindDescriptor(sbdIdx, dbg.vkSet());

        if (casted->cdb == nullptr) {
            casted->cdb = new CachedDiscreteBinding(_STD move(dbg));
        } else {
            casted->cdb->operator=(_STD move(dbg));
        }
    }

    /**
     * Invoke Rendering
     */
    cmd.drawIndexed(1, 0, sgr->_indexData.buffer.count(), 0ui32, 0ui32);
}

void RevDepthStaticNode::after(
    const non_owning_rptr<HORenderPass> renderPass_,
    const non_owning_rptr<RenderStagePass> stagePass_
) const {

    SCOPED_STOPWATCH

    const auto& data { renderPass_->state()->data };

    /**
     * Get Command Buffer
     */
    const auto cmdEntry { data.at("RevDepthStaticNode::CommandBuffer"sv) };
    auto& cmd { *_STD static_pointer_cast<CommandBuffer, void>(cmdEntry) };

    /**
     * End Command Buffer
     */
    cmd.end();

    // Commit Secondary to Primary
    {
        /**
         * Get primary Command Buffer
         */
        const auto primaryEntry { data.at("RevDepthStage::CommandBuffer"sv) };
        auto& primary { *_STD static_pointer_cast<CommandBuffer, void>(primaryEntry) };

        /**
         * Submit
         */
        primary.vkCommandBuffer().executeCommands(1ui32, &cmd.vkCommandBuffer());
    }
}

void RevDepthStaticNode::setupShader(cref<sptr<Device>> device_) {

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
        1,
        shader::BindingUpdateInterval::ePerFrame,
        "staticDepthPassUbo"
    };
    shader::PrototypeBinding mubo {
        shader::BindingType::eStorageBuffer,
        2,
        shader::BindingUpdateInterval::ePerInstance,
        "staticDepthPassModel"
    };

    /**
     * Prepare Prototype Shader
     */
    shader::Prototype vertexPrototype { shader::ShaderType::eVertex, "staticDepthPass" };

    auto vertexShaderCode { read_shader_file("resources/shader/depthpass_static.vert.spv") };
    vertexPrototype.storeCode(vertexShaderCode.data(), vertexShaderCode.size());
    vertexPrototype.add(ubo);
    vertexPrototype.add(mubo);

    shader::Prototype fragmentPrototype { shader::ShaderType::eFragment, "staticDepthPass" };

    auto fragmentShaderCode { read_shader_file("resources/shader/depthpass_static.frag.spv") };
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
