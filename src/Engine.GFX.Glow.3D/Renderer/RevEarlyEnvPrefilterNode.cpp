#include "RevEarlyEnvPrefilterNode.hpp"

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

#ifdef _DEBUG
#include <cassert>
#endif

#include <Engine.GFX/VkFixedPipeline.hpp>
#include <Engine.GFX/API/VkTranslate.hpp>
#include <Engine.GFX/Memory/VkAllocator.hpp>
#include <Engine.GFX.Renderer/HORenderPass.hpp>
#include <Engine.GFX.Renderer/RenderPassState.hpp>
#include <Engine.GFX.Renderer/Stage/RenderStagePass.hpp>
#include <Engine.GFX/Shader/DiscreteBindingGroup.hpp>
#include <Engine.GFX/Shader/Factory.hpp>
#include <Engine.GFX/Shader/Prototype.hpp>
#include <Engine.GFX/Shader/PrototypeBinding.hpp>
#include <Engine.GFX/Shader/ShaderStorage.hpp>
#include <Engine.GFX.Renderer/RenderDataToken.hpp>
#include <Engine.Logging/Logger.hpp>

#include "RevDepthSharedNode.hpp"
#include "Heliogrim/StaticGeometryComponent.hpp"
#include "Engine.Common/Math/Coordinates.hpp"
#include <Engine.GFX/Scene/StaticGeometryBatch.hpp>

#include "Engine.GFX/Graphics.hpp"
#include "Engine.GFX/Scene/SkyboxModel.hpp"
#include "Engine.GFX/Texture/TextureFactory.hpp"
#include "Engine.GFX/Texture/Texture.hpp"
#include "Engine.GFX/Texture/VirtualTextureView.hpp"
#include <Engine.Core/Engine.hpp>
#include <Engine.Reflect/Meta/TypeId.hpp>

using namespace hg::engine::gfx::glow::render;
using namespace hg::engine::gfx::render;
using namespace hg;

struct PrefilterPushBlock {
    float roughness;
    u32 samples = 32ui32;
};

RevEarlyEnvPrefilterNode::RevEarlyEnvPrefilterNode() :
    RenderStageNode(),
    _modelTypes(
        {
            reflect::typeId<SkyboxModel>()
        }
    ),
    _envPrefilterExtent(512ui32, 512ui32),
    _envPrefilterFormat(TextureFormat::eR16G16B16A16Sfloat) {}

void RevEarlyEnvPrefilterNode::setup(cref<sptr<Device>> device_) {
    SCOPED_STOPWATCH

    /**
     * Store device
     */
    _device = device_;

    // LORenderPass
    setupLORenderPass();

    // Shader
    setupShader(device_);

    // Pipeline
    setupPipeline();
}

void RevEarlyEnvPrefilterNode::destroy() {
    SCOPED_STOPWATCH

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

bool RevEarlyEnvPrefilterNode::allocate(const ptr<HORenderPass> renderPass_) {
    SCOPED_STOPWATCH

    const auto state { renderPass_->state() };

    /**
     * Allocate Command Buffer
     */
    auto cmd { _device->graphicsQueue()->pool()->make() };

    /**
     * Create Framebufferr
     */
    Framebuffer framebuffer { allocateFramebuffer() };

    /**
     * Create Prefilter Cube
     */
    const auto* factory { TextureFactory::get() };
    auto prefiltered {
        factory->build(
            {
                math::uivec3 { _envPrefilterExtent, 1ui32 },
                _envPrefilterFormat,
                static_cast<u32>(_STD floorf(_STD log2f(_envPrefilterExtent.x))) + 1ui32,
                TextureType::eCube,
                vk::ImageAspectFlagBits::eColor,
                vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst,
                vk::MemoryPropertyFlagBits::eDeviceLocal,
                vk::SharingMode::eExclusive
            }
        )
    };

    factory->buildView(prefiltered);

    /**
     * Create Uniform Buffers
     */
    Buffer uniform {
        nullptr,
        nullptr,
        _device->vkDevice(),
        sizeof(math::mat4) * 6ui64,
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

    [[maybe_unused]] auto result {
        memory::allocate(&state->alloc, _device, uniform.buffer, MemoryProperty::eHostVisible, uniform.memory)
    };
    uniform.bind();

    // Write matrices to uniform
    #if DROP_VULKAN_CLIP_MATRIX
    Vector<math::mat4> matrices {
        // +x
        math::mat4::make_identity().rotate(glm::radians(90.F), math::vec3_yaw)
                                   .rotate(glm::radians(180.F), math::vec3_pitch),
        // -x
        math::mat4::make_identity().rotate(glm::radians(-90.F), math::vec3_yaw)
                                   .rotate(glm::radians(180.F), math::vec3_pitch),
        // +y
        math::mat4::make_identity().rotate(glm::radians(-90.F), math::vec3_pitch),
        // -y
        math::mat4::make_identity().rotate(glm::radians(90.F), math::vec3_pitch),
        // +z
        math::mat4::make_identity().rotate(glm::radians(180.F), math::vec3_pitch),
        // -z
        math::mat4::make_identity().rotate(glm::radians(180.F), math::vec3_roll)
    };
    #else
    Vector<math::mat4> matrices {
        // +x
        math::mat4::make_identity().rotate(glm::radians(90.F), math::vec3_yaw),
        // -x
        math::mat4::make_identity().rotate(glm::radians(-90.F), math::vec3_yaw),
        // +y
        math::mat4::make_identity().rotate(glm::radians(-90.F), math::vec3_pitch)
                                   .rotate(glm::radians(180.F), math::vec3_roll),
        // -y
        math::mat4::make_identity().rotate(glm::radians(90.F), math::vec3_pitch)
                                   .rotate(glm::radians(180.F), math::vec3_roll),
        // +z
        math::mat4::make_identity().rotate(glm::radians(360.F), math::vec3_yaw),
        // -z
        math::mat4::make_identity().rotate(glm::radians(180.F), math::vec3_yaw),
    };
    #endif

    // Warning: Actualy the vulkan clip matrix, but we suppress the inversion for further usage, so only compress z
    const static math::mat4 clip_matrix = math::mat4(
        1.0F,
        0.0F,
        0.0F,
        0.0F,
        0.0F,
        1.0F,
        0.0F,
        0.0F,
        0.0F,
        0.0F,
        0.5F,
        0.F,
        0.0F,
        0.0F,
        0.5F,
        1.0F
    );

    for (auto& matrix : matrices) {
        matrix = clip_matrix * math::perspective(static_cast<float>(math::pi / 2.0), 1.0F, 0.1F, 512.F) * matrix;
    }

    uniform.write<math::mat4>(matrices.data(), 6ui32);

    /**
     * Warning: Temporary Descriptors
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
    dbgs[0].getById(shader::ShaderBinding::id_type { 1 }).store(uniform);
    // dbgs[1].getById(shader::ShaderBinding::id_type { 2 }).storeAs(..., vk::ImageLayout::eShaderReadOnlyOptimal);

    /**
     * Store State
     */
    renderPass_->state()->data.insert_or_assign(
        "RevEarlyEnvPrefilterNode::CommandBuffer"sv,
        make_sptr<decltype(cmd)>(_STD move(cmd))
    );
    renderPass_->state()->data.insert_or_assign(
        "RevEarlyEnvPrefilterNode::Framebuffer"sv,
        make_sptr<decltype(framebuffer)>(_STD move(framebuffer))
    );
    renderPass_->state()->data.insert_or_assign(
        "RevEarlyEnvPrefilterNode::PrefilterCube"sv,
        make_sptr<decltype(prefiltered)>(_STD move(prefiltered))
    );
    renderPass_->state()->data.insert_or_assign(
        "RevEarlyEnvPrefilterNode::UniformBuffer"sv,
        make_sptr<decltype(uniform)>(_STD move(uniform))
    );

    renderPass_->state()->data.insert_or_assign(
        "RevEarlyEnvPrefilterNode::DiscreteBindingGroups"sv,
        make_sptr<decltype(dbgs)>(_STD move(dbgs))
    );
    renderPass_->state()->data.insert_or_assign(
        "RevEarlyEnvPrefilterNode::DescriptorPools"sv,
        make_sptr<decltype(pools)>(_STD move(pools))
    );

    //
    postProcessAllocated(renderPass_);

    //
    return true;
}

bool RevEarlyEnvPrefilterNode::free(const ptr<HORenderPass> renderPass_) {
    SCOPED_STOPWATCH

    const auto state { renderPass_->state() };

    /**
     *
     */
    auto it { state->data.find("RevEarlyEnvPrefilterNode::CommandBuffer"sv) };
    if (it != state->data.end()) {
        auto cmd { _STD static_pointer_cast<CommandBuffer, void>(it->second) };

        if (cmd->vkCommandBuffer()) {
            auto& lck { cmd->lck() };
            lck.acquire();
            cmd->release();
            lck.release();
        }

        //
        state->data.erase(it);
    }

    /**
     *
     */
    it = state->data.find("RevEarlyEnvPrefilterNode::Framebuffer"sv);
    if (it != state->data.end()) {
        auto framebuffer { _STD static_pointer_cast<Framebuffer, void>(it->second) };
        freeFramebuffer(_STD move(*framebuffer));

        //
        state->data.erase(it);
    }

    /**
     *
     */
    it = state->data.find("RevEarlyEnvPrefilterNode::PrefilterCube"sv);
    if (it != state->data.end()) {
        auto prefiltered { _STD static_pointer_cast<Texture, void>(it->second) };
        prefiltered->destroy();

        //
        state->data.erase(it);
    }

    /**
     *
     */
    it = state->data.find("RevEarlyEnvPrefilterNode::UniformBuffer"sv);
    if (it != state->data.end()) {
        auto uniform { _STD static_pointer_cast<Buffer, void>(it->second) };
        uniform->destroy();

        //
        state->data.erase(it);
    }

    /**
     *
     */
    it = state->data.find("RevEarlyEnvPrefilterNode::DiscreteBindingGroups"sv);
    if (it != state->data.end()) {
        sptr<Vector<shader::DiscreteBindingGroup>> dbgs {
            _STD static_pointer_cast<Vector<shader::DiscreteBindingGroup>, void>(it->second)
        };

        const auto poolIt { state->data.find("RevEarlyEnvPrefilterNode::DescriptorPools"sv) };
        sptr<Vector<vk::DescriptorPool>> pools {
            _STD static_pointer_cast<Vector<vk::DescriptorPool>, void>(poolIt->second)
        };

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
     *
     */
    it = state->data.find("RevEarlyEnvPrefilterNode::LastRecordedActor"sv);
    if (it != state->data.end()) {
        sptr<ptr<void>> dbgs {
            _STD static_pointer_cast<ptr<void>, void>(it->second)
        };

        /**/
        state->data.erase(it);
    }

    state->data.erase("RevEarlyEnvPrefilterNode::ShouldUpdate"sv);

    return true;
}

Vector<RenderDataToken> RevEarlyEnvPrefilterNode::requiredToken() noexcept {
    return {};
}

Vector<RenderDataToken> RevEarlyEnvPrefilterNode::optionalToken() noexcept {
    return {};
}

const non_owning_rptr<const Vector<type_id>> RevEarlyEnvPrefilterNode::modelTypes() const noexcept {
    return &_modelTypes;
}

void RevEarlyEnvPrefilterNode::before(
    const non_owning_rptr<HORenderPass> renderPass_,
    const non_owning_rptr<RenderStagePass> stagePass_
) const {
    SCOPED_STOPWATCH

    auto* state { renderPass_->state().get() };
    const auto& data { state->data };
}

void RevEarlyEnvPrefilterNode::invoke(
    const non_owning_rptr<HORenderPass> renderPass_,
    const non_owning_rptr<RenderStagePass> stagePass_,
    const non_owning_rptr<SceneNodeModel> model_
) const {
    SCOPED_STOPWATCH

    auto* state { renderPass_->state().get() };
    auto& data { state->data };

    /**
     *
     */
    const auto* model { static_cast<const ptr<SkyboxModel>>(model_) };
    ptr<void> lastOwner { nullptr };

    const auto lastActorEntry { data.find("RevEarlyEnvPrefilterNode::LastRecordedActor"sv) };
    if (lastActorEntry != data.end()) {
        lastOwner = *_STD static_pointer_cast<ptr<void>, void>(lastActorEntry->second);
    }

    const bool shouldUpdate { lastOwner != model->owner() };

    if (shouldUpdate) {
        data.insert_or_assign("RevEarlyEnvPrefilterNode::ShouldUpdate"sv, nullptr);
    } else {
        data.erase("RevEarlyEnvPrefilterNode::ShouldUpdate"sv);
    }

    /**
     *
     */
    if (shouldUpdate) {
        /**
         * Prepare Command Bufferr
         */
        const auto cmdEntry { data.at("RevEarlyEnvPrefilterNode::CommandBuffer"sv) };
        auto& cmd { *_STD static_pointer_cast<CommandBuffer, void>(cmdEntry) };

        cmd.reset();
        cmd.begin();

        /**
         * Bind Skybox Resources
         */
        if (model->hasOverrideMaterials()) {
            const auto& first { model->overrideMaterials().front() };
            auto guard = first->acquire(resource::ResourceUsageFlag::eRead);

            ptr<const VirtualTextureView> skyboxView { nullptr };
            if (false/* TODO: Check whether material has diffuse/albedo texture */) {

                // TODO: Resolve texture to use for irradiance generator
                // TODO: Bind image to descriptor set
                assert(false);

            } else {
                IM_DEBUG_LOG("Irradiance generator picked up a skybox instance without satisfying resources.");
            }
        }

        /**
         * Get Prefilter Cube
         */
        const auto prefilterEntry { data.at("RevEarlyEnvPrefilterNode::PrefilterCube"sv) };
        auto& prefiltered { *_STD static_pointer_cast<Texture, void>(prefilterEntry) };

        /**
         * Transform prefiltered to get stored data
         */
        vk::ImageMemoryBarrier toTransDstLayout {
            vk::AccessFlagBits::eShaderRead,
            vk::AccessFlagBits::eTransferWrite,
            vk::ImageLayout::eShaderReadOnlyOptimal,
            vk::ImageLayout::eTransferDstOptimal,
            0,
            0,
            prefiltered.buffer().image(),
            vk::ImageSubresourceRange {
                vk::ImageAspectFlagBits::eColor,
                0ui32,
                prefiltered.mipLevels(),
                0ui32,
                6ui32
            }
        };

        cmd.vkCommandBuffer().pipelineBarrier(
            vk::PipelineStageFlagBits::eAllGraphics,
            vk::PipelineStageFlagBits::eTransfer,
            vk::DependencyFlagBits::eByRegion,
            0ui32,
            nullptr,
            0,
            nullptr,
            1ui32,
            &toTransDstLayout
        );

        /**
         *
         */
        PrefilterPushBlock pushBlock { 0.F };

        /**
         * Loop for every mip level to prefilter/sample to the proxy texture and then transfer to the target cube map
         */
        for (auto mip { 0ui32 }; mip < prefiltered.mipLevels(); ++mip) {
            /**
             *
             */
            pushBlock.roughness = static_cast<float>(mip) / static_cast<float>(prefiltered.mipLevels() - 1);

            /**
             * Begin LORenderPass :: Inline Content
             */
            const auto frameEntry { data.at("RevEarlyEnvPrefilterNode::Framebuffer"sv) };
            auto& framebuffer { *_STD static_pointer_cast<Framebuffer, void>(frameEntry) };

            cmd.beginRenderPass(*_loRenderPass, framebuffer, true);

            /**
             * Pipeline
             */
            Viewport viewport {
                _envPrefilterExtent.x >> mip,
                _envPrefilterExtent.y >> mip
            };

            cmd.bindPipeline(_pipeline.get(), viewport);

            /**
             * Descriptor
             */
            auto dbgs {
                _STD static_pointer_cast<Vector<shader::DiscreteBindingGroup>, void>(
                    data.find("RevEarlyEnvPrefilterNode::DiscreteBindingGroups"sv)->second
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

            /**
             * Push Blocks
             */
            cmd.vkCommandBuffer().pushConstants(
                static_cast<ptr<VkFixedPipeline>>(_pipeline.get())->vkLayout(),
                vk::ShaderStageFlagBits::eFragment,
                0,
                sizeof(PrefilterPushBlock),
                &pushBlock
            );

            /**
             * Dispatch
             */
            cmd.draw(1ui32, 0, 36ui32, 0);

            cmd.endRenderPass();

            /**
             * Copy sampled mip data
             */

            // Prepare framebuffer attachment to transfer from framebuffer to prefiltered
            vk::ImageMemoryBarrier toTransLayout {
                vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite,
                vk::AccessFlagBits::eTransferRead,
                vk::ImageLayout::eColorAttachmentOptimal,
                vk::ImageLayout::eTransferSrcOptimal,
                0,
                0,
                framebuffer.attachments()[0].unwrapped()->buffer().image(),
                vk::ImageSubresourceRange {
                    vk::ImageAspectFlagBits::eColor,
                    0ui32,
                    1ui32,
                    0ui32,
                    6ui32
                }
            };

            cmd.vkCommandBuffer().pipelineBarrier(
                vk::PipelineStageFlagBits::eColorAttachmentOutput,
                vk::PipelineStageFlagBits::eTransfer,
                vk::DependencyFlagBits::eByRegion,
                0ui32,
                nullptr,
                0,
                nullptr,
                1ui32,
                &toTransLayout
            );

            // Copy current sampled data to target mip level of prefiltered map
            vk::ImageCopy copy {
                vk::ImageSubresourceLayers {
                    vk::ImageAspectFlagBits::eColor,
                    0ui32,
                    0ui32,
                    6ui32
                },
                vk::Offset3D {},
                vk::ImageSubresourceLayers {
                    vk::ImageAspectFlagBits::eColor,
                    mip,
                    0ui32,
                    6ui32
                },
                vk::Offset3D {},
                vk::Extent3D { viewport.width(), viewport.height(), 1ui32 }
            };

            cmd.vkCommandBuffer().copyImage(
                framebuffer.attachments()[0].unwrapped()->buffer().image(),
                vk::ImageLayout::eTransferSrcOptimal,
                prefiltered.buffer().image(),
                vk::ImageLayout::eTransferDstOptimal,
                1,
                &copy
            );

            // Transform framebuffer attachment layout back to get written by the render pass
            vk::ImageMemoryBarrier toSampleLayout {
                vk::AccessFlagBits::eTransferRead,
                vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite,
                vk::ImageLayout::eTransferSrcOptimal,
                vk::ImageLayout::eColorAttachmentOptimal,
                0,
                0,
                framebuffer.attachments()[0].unwrapped()->buffer().image(),
                vk::ImageSubresourceRange {
                    vk::ImageAspectFlagBits::eColor,
                    0ui32,
                    1ui32,
                    0ui32,
                    6ui32
                }
            };

            cmd.vkCommandBuffer().pipelineBarrier(
                vk::PipelineStageFlagBits::eTransfer,
                vk::PipelineStageFlagBits::eAllGraphics,
                vk::DependencyFlagBits::eByRegion,
                0ui32,
                nullptr,
                0,
                nullptr,
                1ui32,
                &toSampleLayout
            );
        }

        /**
         * Transform prefiltered to sampled layout
         */
        vk::ImageMemoryBarrier toSampleLayout {
            vk::AccessFlagBits::eTransferWrite,
            vk::AccessFlagBits::eShaderRead,
            vk::ImageLayout::eTransferDstOptimal,
            vk::ImageLayout::eShaderReadOnlyOptimal,
            0,
            0,
            prefiltered.buffer().image(),
            vk::ImageSubresourceRange {
                vk::ImageAspectFlagBits::eColor,
                0ui32,
                prefiltered.mipLevels(),
                0ui32,
                6ui32
            }
        };

        cmd.vkCommandBuffer().pipelineBarrier(
            vk::PipelineStageFlagBits::eTransfer,
            vk::PipelineStageFlagBits::eFragmentShader,
            vk::DependencyFlagBits::eByRegion,
            0ui32,
            nullptr,
            0,
            nullptr,
            1ui32,
            &toSampleLayout
        );

        //
        data.insert_or_assign("RevEarlyEnvPrefilterNode::LastRecordedActor"sv, make_sptr<ptr<void>>(model->owner()));
    }
}

void RevEarlyEnvPrefilterNode::after(
    const non_owning_rptr<HORenderPass> renderPass_,
    const non_owning_rptr<RenderStagePass> stagePass_
) const {
    SCOPED_STOPWATCH

    const auto& data { renderPass_->state()->data };

    const bool shouldUpdate { data.find("RevEarlyEnvPrefilterNode::ShouldUpdate"sv) != data.end() };

    if (shouldUpdate) {
        /**
         * Get Command Buffer
         */
        const auto cmdEntry { data.at("RevEarlyEnvPrefilterNode::CommandBuffer"sv) };
        auto& cmd { *_STD static_pointer_cast<CommandBuffer, void>(cmdEntry) };

        /**
         * End Command Buffer
         */
        cmd.end();

        /**
         * Submit Command Buffer to Command Batch
         */
        stagePass_->batch().push(cmd);
    }
}

void RevEarlyEnvPrefilterNode::setupShader(cref<sptr<Device>> device_) {
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
        "envPrefilterUbo"
    };
    shader::PrototypeBinding cube {
        shader::BindingType::eImageSampler,
        2,
        shader::BindingUpdateInterval::eMaterialUpdate,
        "envPrefilterCube"
    };

    /**
     * Prepare Prototype Shader
     */
    shader::Prototype vertexPrototype { shader::ShaderType::eVertex, "envPrefilterGen" };

    auto vertexShaderCode { read_shader_file("resources/shader/envPrefilter.gen.vert.spv") };
    vertexPrototype.storeCode(vertexShaderCode.data(), vertexShaderCode.size());
    vertexPrototype.add(ubo);

    shader::Prototype fragmentPrototype { shader::ShaderType::eFragment, "envPrefilterGen" };

    auto fragmentShaderCode { read_shader_file("resources/shader/envPrefilter.gen.frag.spv") };
    fragmentPrototype.storeCode(fragmentShaderCode.data(), fragmentShaderCode.size());
    fragmentPrototype.add(cube);

    /**
     * Build Shader and Bindings
     */
    auto factoryResult {
        shaderFactory.build(
            {
                vertexPrototype,
                fragmentPrototype
            }
        )
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
            vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind,
            1ui32,
            static_cast<u32>(sizes.size()),
            sizeMem
        };

        _requiredDescriptorPools.push_back(dpci);
        _requiredBindingGroups.push_back(group);
    }
}

void RevEarlyEnvPrefilterNode::setupLORenderPass() {
    /**
     * LORenderPass
     */
    _loRenderPass = make_sptr<pipeline::LORenderPass>(_device);

    // Cube Attachment
    _loRenderPass->set(
        0,
        vk::AttachmentDescription {
            vk::AttachmentDescriptionFlags(),
            api::vkTranslateFormat(_envPrefilterFormat),
            vk::SampleCountFlagBits::e1,
            vk::AttachmentLoadOp::eClear,
            vk::AttachmentStoreOp::eStore,
            vk::AttachmentLoadOp::eDontCare,
            vk::AttachmentStoreOp::eDontCare,
            vk::ImageLayout::eUndefined,
            vk::ImageLayout::eColorAttachmentOptimal
        }
    );

    _loRenderPass->_viewMasks.push_back(0b0000'0000'0011'1111ui32);
    _loRenderPass->_correlationMasks.push_back(0ui32);

    /**
     *
     */
    _loRenderPass->setup();

    // Post modify clear values
    const_cast<ptr<vk::ClearValue>>(_loRenderPass->vkBeginInfo().pClearValues)[0].color.float32[3] = 0.2F;
}

void RevEarlyEnvPrefilterNode::destroyLORenderPass() {
    if (_loRenderPass) {
        _loRenderPass->destroy();
        _loRenderPass.reset();
    }
}

void RevEarlyEnvPrefilterNode::setupPipeline() {
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

    _pipeline->vertexStage().shaderSlot().name() = "envPrefilterGen";
    _pipeline->fragmentStage().shaderSlot().name() = "envPrefilterGen";

    // Warning: Actualy 'RasterCullFace::eFront' but we suppress the vulkan clip matrix, so we swap the polys facing direction
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

    blending.push_back(colorState);

    static_cast<ptr<VkFixedPipeline>>(_pipeline.get())->pushConstants().push_back(
        {
            vk::ShaderStageFlagBits::eFragment,
            0ui32,
            sizeof(PrefilterPushBlock)
        }
    );

    /**
     *
     */
    _pipeline->setup();
}

[[nodiscard]] engine::gfx::Framebuffer RevEarlyEnvPrefilterNode::allocateFramebuffer() {
    /**
     * Create Framebuffer
     */
    const auto* factory { TextureFactory::get() };

    Framebuffer buffer { _device };
    buffer.setExtent(math::uivec3(_envPrefilterExtent, 1ui32));
    buffer.setRenderPass(_loRenderPass);

    /**
     * Create Framebuffer :: Attachments
     */
    auto prefilter = factory->build(
        {
            buffer.extent(),
            _envPrefilterFormat,
            // Prefilter Sample Target
            1ui32,
            TextureType::eCube,
            vk::ImageAspectFlagBits::eColor,
            vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc,
            vk::MemoryPropertyFlagBits::eDeviceLocal,
            vk::SharingMode::eExclusive
        }
    );

    factory->buildView(prefilter);

    /**
     * Create Framebuffer :: Attach
     */
    buffer.add({ _STD move(prefilter) });

    /**
     *
     */
    buffer.setup();

    return buffer;
}

void RevEarlyEnvPrefilterNode::freeFramebuffer(mref<Framebuffer> framebuffer_) {
    auto fb { _STD move(framebuffer_) };

    //
    fb.destroy();

    for (auto& attachment : fb.attachments()) {
        attachment->destroy();
    }
}

void RevEarlyEnvPrefilterNode::postProcessAllocated(const ptr<HORenderPass> renderPass_) const {
    auto& data { renderPass_->state()->data };

    const auto frameEntry { data.at("RevEarlyEnvPrefilterNode::Framebuffer"sv) };
    auto& framebuffer { *_STD static_pointer_cast<Framebuffer, void>(frameEntry) };

    Vector<vk::ImageMemoryBarrier> imgBarriers {};
    for (const auto& entry : framebuffer.attachments()) {
        auto& attachment { *entry.unwrapped() };

        if (isDepthFormat(attachment.format()) || isStencilFormat(attachment.format())) {
            continue;
        }

        imgBarriers.push_back(
            {
                vk::AccessFlags {},
                vk::AccessFlagBits::eColorAttachmentWrite,
                vk::ImageLayout::eUndefined,
                vk::ImageLayout::eColorAttachmentOptimal,
                VK_QUEUE_FAMILY_IGNORED,
                VK_QUEUE_FAMILY_IGNORED,
                attachment.buffer().image(),
                vk::ImageSubresourceRange {
                    vk::ImageAspectFlagBits::eColor,
                    0,
                    attachment.mipLevels(),
                    0,
                    attachment.layer()
                }
            }
        );
    }

    const auto prefilterEntry { data.at("RevEarlyEnvPrefilterNode::PrefilterCube"sv) };
    auto& prefiltered { *_STD static_pointer_cast<Texture, void>(prefilterEntry) };

    imgBarriers.push_back(
        {
            vk::AccessFlags {},
            vk::AccessFlagBits::eShaderRead,
            vk::ImageLayout::eUndefined,
            vk::ImageLayout::eShaderReadOnlyOptimal,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            prefiltered.buffer().image(),
            vk::ImageSubresourceRange {
                vk::ImageAspectFlagBits::eColor,
                0,
                prefiltered.mipLevels(),
                0,
                prefiltered.layer()
            }
        }
    );

    auto pool = _device->graphicsQueue()->pool();
    pool->lck().acquire();
    CommandBuffer cmd = pool->make();
    cmd.begin();

    /**
     * Transform
     */
    cmd.vkCommandBuffer().pipelineBarrier(
        vk::PipelineStageFlagBits::eAllCommands,
        vk::PipelineStageFlagBits::eAllCommands,
        vk::DependencyFlags {},
        0,
        nullptr,
        0,
        nullptr,
        static_cast<u32>(imgBarriers.size()),
        imgBarriers.data()
    );

    cmd.end();
    cmd.submitWait();
    cmd.release();

    pool->lck().release();
}
