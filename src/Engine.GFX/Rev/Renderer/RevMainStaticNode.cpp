#include "RevMainStaticNode.hpp"

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

#ifdef _DEBUG
#include <cassert>
#endif

#include <Ember/StaticGeometryComponent.hpp>
#include <Engine.GFX/VkFixedPipeline.hpp>
#include <Engine.GFX/API/VkTranslate.hpp>
#include <Engine.GFX/Command/CommandBuffer.hpp>
#include <Engine.GFX/Memory/VkAllocator.hpp>
#include <Engine.GFX/Renderer/HORenderPass.hpp>
#include <Engine.GFX/Renderer/RenderPassState.hpp>
#include <Engine.GFX/Renderer/RenderStagePass.hpp>
#include <Engine.GFX/Resource/StaticGeometryResource.hpp>
#include <Engine.GFX/Shader/DiscreteBindingGroup.hpp>
#include <Engine.GFX/Shader/Factory.hpp>
#include <Engine.GFX/Shader/Prototype.hpp>
#include <Engine.GFX/Shader/PrototypeBinding.hpp>
#include <Engine.GFX/Shader/ShaderStorage.hpp>

#include "RevMainSharedNode.hpp"
#include "__macro.hpp"
#include "Engine.Common/Math/Coordinates.hpp"
#include "Engine.GFX/Loader/RevTextureLoader.hpp"
#include "Engine.GFX/Scene/StaticGeometryModel.hpp"
#include "Engine.GFX/Texture/TextureFactory.hpp"
#include "Engine.Reflect/EmberReflect.hpp"

using namespace ember::engine::gfx::render;
using namespace ember::engine::gfx;
using namespace ember;

static Texture testAlbedo {};
static Texture testNormal {};
static Texture testRoughness {};
static Texture testSpecular {};

RevMainStaticNode::RevMainStaticNode(const ptr<RevMainSharedNode> sharedNode_) :
    RenderStageNode(),
    _modelTypes({
        EmberClass::stid<StaticGeometryModel>()
    }),
    _sharedNode(sharedNode_) {}

void RevMainStaticNode::setup(cref<sptr<Device>> device_) {

    SCOPED_STOPWATCH

    /**
     * Store device
     */
    _device = device_;

    /**
     *
     */
    setupShader();

    /**
     * Fixed Pipeline
     */
    _pipeline = make_sptr<VkFixedPipeline>(device_, _sharedNode->loRenderPass());
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
    _pipeline->rasterizationStage().depthWrite() = true;// TODO: Revert to `false`, currently true for debugging purpose

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
        RevTextureLoader loader { _device };
        testAlbedo = loader.__tmp__load({ ""sv, R"(R:\\albedo.ktx)"sv });
        testNormal = loader.__tmp__load({ ""sv, R"(R:\\normal.ktx)"sv });
        testRoughness = loader.__tmp__load({ ""sv, R"(R:\\roughness.ktx)"sv });
        testSpecular = loader.__tmp__load({ ""sv, R"(R:\\specular.ktx)"sv });

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

void RevMainStaticNode::destroy() {

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
        _device->vkDevice().destroyDescriptorSetLayout(entry.vkSetLayout());
    }
}

bool RevMainStaticNode::allocate(const ptr<HORenderPass> renderPass_) {

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

    ptr<VkAllocator> alloc {
        VkAllocator::makeForBuffer(_device, uniform.buffer, vk::MemoryPropertyFlagBits::eHostVisible)
    };

    const vk::MemoryRequirements req { _device->vkDevice().getBufferMemoryRequirements(uniform.buffer) };
    uniform.memory = alloc->allocate(req.size);
    uniform.bind();

    delete alloc;

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
    dbgs[2].getById(shader::ShaderBinding::id_type { 3 }).store(testAlbedo);
    dbgs[2].getById(shader::ShaderBinding::id_type { 4 }).store(testNormal);
    dbgs[2].getById(shader::ShaderBinding::id_type { 5 }).store(testRoughness);
    dbgs[2].getById(shader::ShaderBinding::id_type { 6 }).store(testSpecular);

    /**
     * Store State
     */
    state->data.insert_or_assign("RevMainStaticNode::CommandBuffer"sv,
        _STD make_shared<decltype(cmd)>(_STD move(cmd)));
    state->data.insert_or_assign("RevMainStaticNode::UniformBuffer"sv,
        _STD make_shared<decltype(uniform)>(_STD move(uniform)));
    state->data.insert_or_assign("RevMainStaticNode::DiscreteBindingGroups"sv,
        _STD make_shared<decltype(dbgs)>(_STD move(dbgs)));
    state->data.insert_or_assign("RevMainStaticNode::DescriptorPools"sv,
        _STD make_shared<decltype(pools)>(_STD move(pools)));

    return true;
}

bool RevMainStaticNode::free(const ptr<HORenderPass> renderPass_) {

    SCOPED_STOPWATCH

    const auto state { renderPass_->state() };

    /**
     * Free Descriptors
     */
    auto it { state->data.find("RevMainStaticNode::DiscreteBindingGroups"sv) };
    if (it != state->data.end()) {

        sptr<Vector<shader::DiscreteBindingGroup>> dbgs {
            _STD static_pointer_cast<Vector<shader::DiscreteBindingGroup>, void>(it->second)
        };

        const auto poolIt { state->data.find("RevMainStaticNode::DescriptorPools"sv) };
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
            _device->vkDevice().freeDescriptorSets(pool, 1ui32, &dbg.vkSet())
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
    it = state->data.find("RevMainStaticNode::UniformBuffer"sv);
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
    it = renderPass_->state()->data.find("RevMainStaticNode::CommandBuffer"sv);
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

const non_owning_rptr<const Vector<type_id>> RevMainStaticNode::modelTypes() const noexcept {
    return &_modelTypes;
}

void RevMainStaticNode::before(
    const non_owning_rptr<HORenderPass> renderPass_,
    const non_owning_rptr<RenderStagePass> stagePass_
) const {

    SCOPED_STOPWATCH

    const auto& data { renderPass_->state()->data };

    /**
     * Prepare Command Buffer
     */
    const auto cmdEntry { data.at("RevMainStaticNode::CommandBuffer"sv) };
    auto& cmd { *_STD static_pointer_cast<CommandBuffer, void>(cmdEntry) };

    const auto entry { data.at("RevMainStage::Framebuffer"sv) };
    auto& frame { *_STD static_pointer_cast<Framebuffer, void>(entry) };

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
     * Update Resources [BindingUpdateInterval::ePerFrame]
     */
    const auto uniformEntry { data.at("RevMainStaticNode::UniformBuffer"sv) };
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
            data.find("RevMainStaticNode::DiscreteBindingGroups"sv)->second
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

void RevMainStaticNode::invoke(
    const non_owning_rptr<HORenderPass> renderPass_,
    const non_owning_rptr<RenderStagePass> stagePass_,
    const non_owning_rptr<SceneNodeModel> model_
) const {

    SCOPED_STOPWATCH

    const auto& data { renderPass_->state()->data };

    /**
     * Get Command Buffer
     */
    const auto cmdEntry { data.at("RevMainStaticNode::CommandBuffer"sv) };
    auto& cmd { *_STD static_pointer_cast<CommandBuffer, void>(cmdEntry) };

    // Temporary
    const auto* owner { static_cast<const ptr<StaticGeometryComponent>>(model_->owner()) };
    const auto* model { static_cast<const ptr<StaticGeometryModel>>(model_) };
    if (!model->geometryResource()->isLoaded()) {
        return;
    }

    const auto* res { static_cast<const ptr<StaticGeometryResource>>(model->geometryResource()) };

    /**
     *
     */
    cmd.bindVertexBuffer(0, res->_vertexData.buffer, 0);
    cmd.bindIndexBuffer(res->_indexData.buffer, 0);

    // Temporary
    if (!model->__tmp__instance.memory) {
        auto& buffer { const_cast<ptr<StaticGeometryModel>>(model)->__tmp__instance };
        buffer.device = _device->vkDevice();
        buffer.size = static_cast<u64>(sizeof(math::mat4));
        buffer.usageFlags = vk::BufferUsageFlagBits::eStorageBuffer;

        const vk::BufferCreateInfo ci { {}, buffer.size, buffer.usageFlags, vk::SharingMode::eExclusive, 0, nullptr };
        buffer.buffer = _device->vkDevice().createBuffer(ci);
        assert(buffer.buffer);

        ptr<VkAllocator> alloc {
            VkAllocator::makeForBuffer(_device, buffer.buffer, vk::MemoryPropertyFlagBits::eHostVisible)
        };

        const vk::MemoryRequirements req { _device->vkDevice().getBufferMemoryRequirements(buffer.buffer) };
        buffer.memory = alloc->allocate(req.size);
        buffer.bind();

        delete alloc;

        /**
         * Push Data
         */
        auto translation {
            math::mat4::make_identity().translate(
                owner->getWorldTransform().position()
            )
        };

        auto scale {
            math::mat4::make_identity().unchecked_scale(
                owner->getWorldTransform().scale()
            )
        };

        // TODO: auto rotation { as<quaternion, mat4>(owner->getWorldTransform().rotation()) };

        auto euler { owner->getWorldTransform().rotation().euler() };
        auto rotation { math::mat4::make_identity() };
        rotation.rotate(euler.x, math::vec3_pitch);
        rotation.rotate(euler.y, math::vec3_yaw);
        rotation.rotate(euler.z, math::vec3_roll);

        const auto mm { translation * rotation * scale };
        buffer.write<math::mat4>(&mm, 1ui32);
    }

    // TODO: Optimize finding binding group for model update [ePerInstance]
    u32 sbgIdx { 0ui32 };
    ptr<const shader::ShaderBindingGroup> sbg { nullptr };
    for (; sbgIdx < _requiredBindingGroups.size(); ++sbgIdx) {
        const auto& entry { _requiredBindingGroups[sbgIdx] };
        if (entry.interval() == shader::BindingUpdateInterval::ePerInstance) {
            sbg = &entry;
            break;
        }
    }

    // Trigger early exit if no suitable shader binding group was found
    if (sbg == nullptr) {
        return;
    }

    if (model->__tmp__cdb != nullptr) {
        auto& dbg { model->__tmp__cdb->binding() };
        //dbg.getById(2ui32).store(model->__tmp__instance);
        cmd.bindDescriptor(sbgIdx, dbg.vkSet());

    } else {
        /**
         * Acquire Descriptor for model data, push data and bind it
         */
        auto& cache { renderPass_->state()->bindingCache };
        auto dbg { cache.allocate(*sbg) };

        dbg.getById(2ui32).store(model->__tmp__instance);
        cmd.bindDescriptor(sbgIdx, dbg.vkSet());

        // TODO: Replace storing of cached descriptor binding
        if (model->__tmp__cdb == nullptr) {
            // const_cast<_STD remove_cv_t<_STD remove_pointer_t<decltype(model)>>*>(model);
            const_cast<StaticGeometryModel*>(model)->__tmp__cdb = new CachedDiscreteBinding { _STD move(dbg) };
        } else {
            model->__tmp__cdb->operator=(_STD move(dbg));
        }

    }

    /**
     * Invoke Rendering Code
     */
    cmd.drawIndexed(1, 0, res->_indexData.buffer.count(), 0ui32, 0ui32);
}

void RevMainStaticNode::after(
    const non_owning_rptr<HORenderPass> renderPass_,
    const non_owning_rptr<RenderStagePass> stagePass_
) const {

    SCOPED_STOPWATCH

    const auto& data { renderPass_->state()->data };

    /**
     * Get Command Buffer
     */
    const auto cmdEntry { data.at("RevMainStaticNode::CommandBuffer"sv) };
    auto& cmd { *_STD static_pointer_cast<CommandBuffer, void>(cmdEntry) };

    /**
     * End Command Buffer
     */
    cmd.end();

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

void RevMainStaticNode::setupShader() {

    /**
     *
     */
    shader::Factory shaderFactory { _device };
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
        shader::BindingUpdateInterval::ePerInstance,
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
