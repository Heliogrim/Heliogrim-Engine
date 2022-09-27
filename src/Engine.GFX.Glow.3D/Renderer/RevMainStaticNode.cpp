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
#include <Engine.GFX/Renderer/RenderDataToken.hpp>
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
#include "Engine.GFX/Graphics.hpp"
#include "Engine.GFX/Loader/RevTextureLoader.hpp"
#include "Engine.GFX/Performance/RenderTokens.hpp"
#include "Engine.GFX/Scene/ModelDataTokens.hpp"
#include "Engine.GFX/Scene/StaticGeometryBatch.hpp"
#include "Engine.GFX/Scene/StaticGeometryModel.hpp"
#include "Engine.GFX/Texture/TextureFactory.hpp"
#include <Engine.GFX.Glow/Texture/RevVirtualMarkerTexture.hpp>
#include "Engine.Reflect/EmberReflect.hpp"
#include "State/RevSfMtt.hpp"

using namespace ember::engine::gfx::glow::render;
using namespace ember::engine::gfx::render;
using namespace ember::engine::gfx;
using namespace ember;

#define MATERIAL_DESCRIPTOR_INDEX 2

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
            { 3ui32, TextureFormat::eR32G32B32Sfloat, static_cast<u32>(offsetof(vertex, normal)) },
            { 4ui32, TextureFormat::eR32G32B32Sfloat, static_cast<u32>(offsetof(vertex, tangent)) }
        }
    });

    _pipeline->vertexStage().shaderSlot().name() = "staticMainPass";
    _pipeline->fragmentStage().shaderSlot().name() = "staticMainPass";

    _pipeline->rasterizationStage().cullFace() = RasterCullFace::eBack;
    // TODO: Default => RasterCullFace::eBack | Foliage => RasterCullFace::eNone
    //_pipeline->rasterizationStage().depthWrite() = true;// TODO: Revert to `false`, currently true for debugging purpose
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

    const auto result {
        memory::allocate(&state->alloc, _device, uniform.buffer, MemoryProperty::eHostVisible, uniform.memory)
    };// TODO: Handle failed allocation
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

        if (_requiredBindingGroups[rdp].interval() == shader::BindingUpdateInterval::eMaterialUpdate) {
            continue;
        }

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
    state->data.insert_or_assign("RevMainStaticNode::CommandBuffer"sv,
        _STD make_shared<decltype(cmd)>(_STD move(cmd)));
    state->data.insert_or_assign("RevMainStaticNode::UniformBuffer"sv,
        _STD make_shared<decltype(uniform)>(_STD move(uniform)));
    state->data.insert_or_assign("RevMainStaticNode::DiscreteBindingGroups"sv,
        _STD make_shared<decltype(dbgs)>(_STD move(dbgs)));
    state->data.insert_or_assign("RevMainStaticNode::DescriptorPools"sv,
        _STD make_shared<decltype(pools)>(_STD move(pools)));

    using material_map_type = ska::bytell_hash_map<ptr<void>, shader::DiscreteBindingGroup>;
    state->data.insert_or_assign("RevMainStaticNode::MaterialDescriptors"sv, _STD make_shared<material_map_type>());

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

    it = state->data.find("RevMainStaticNode::MaterialDescriptors"sv);
    if (it != state->data.end()) {

        using material_map_type = ska::bytell_hash_map<ptr<void>, shader::DiscreteBindingGroup>;
        auto materialDescriptors {
            _STD static_pointer_cast<material_map_type, void>(it->second)
        };

        materialDescriptors->clear();

        /**
         *
         */
        state->data.erase(it);
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

Vector<RenderDataToken> RevMainStaticNode::requiredToken() noexcept {
    return { StaticModelGeometry, StaticModelTransform };
}

Vector<RenderDataToken> RevMainStaticNode::optionalToken() noexcept {
    return { PerformanceQuery };
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

    const static auto clip_matrix = math::mat4(
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
    }

    const_cast<ptr<RevMainStaticNode>>(this)->__test_flag.clear();
}

void RevMainStaticNode::invoke(
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
    const auto cmdEntry { data.at("RevMainStaticNode::CommandBuffer"sv) };
    auto& cmd { *_STD static_pointer_cast<CommandBuffer, void>(cmdEntry) };

    // Temporary
    const auto* model { static_cast<const ptr<StaticGeometryModel>>(model_) };
    if (!model->geometryResource()->isLoaded()) {
        return;
    }

    const auto* res { static_cast<const ptr<StaticGeometryResource>>(model->geometryResource()) };

    if (model->hasOverrideMaterials()) {
        auto* first { model->overrideMaterials().front() };

        using material_map_type = ska::bytell_hash_map<ptr<void>, shader::DiscreteBindingGroup>;

        auto dataEntry { data.find("RevMainStaticNode::MaterialDescriptors"sv) };
        auto materialDescriptors { _STD static_pointer_cast<material_map_type, void>(dataEntry->second) };

        auto materialIter { materialDescriptors->find(const_cast<ptr<assets::Asset>>(first->origin())) };
        if (materialIter == materialDescriptors->end()) {
            // TODO:
            auto dbg { createMaterialDescriptor(state) };

            //
            dbg.getById(shader::ShaderBinding::id_type { 3 }).store(*first->_payload.view.get());

            //
            dbg.getById(shader::ShaderBinding::id_type { 4 }).store(first->_payload.diffuse->_payload.view->owner());

            //
            if (first->_payload.normal) {
                dbg.getById(shader::ShaderBinding::id_type { 5 }).store(
                    first->_payload.normal->_payload.view->owner());
            }
            if (first->_payload.roughness) {
                dbg.getById(shader::ShaderBinding::id_type { 6 }).store(
                    first->_payload.roughness->_payload.view->owner());
            }
            if (first->_payload.metalness) {
                dbg.getById(shader::ShaderBinding::id_type { 7 }).store(
                    first->_payload.metalness->_payload.view->owner());
            }
            if (first->_payload.ao) {
                dbg.getById(shader::ShaderBinding::id_type { 8 }).store(
                    first->_payload.ao->_payload.view->owner());
            }
            if (first->_payload.alpha) {
                dbg.getById(shader::ShaderBinding::id_type { 9 }).store(
                    first->_payload.alpha->_payload.view->owner());
            }
            if (first->_payload.ao) {
                dbg.getById(shader::ShaderBinding::id_type { 10 }).store(
                    first->_payload.ao->_payload.view->owner());
            }

            //
            auto result {
                materialDescriptors->insert_or_assign(const_cast<ptr<assets::Asset>>(first->origin()), _STD move(dbg))
            };
            materialIter = result.first;
        }

        cmd.bindDescriptor(MATERIAL_DESCRIPTOR_INDEX, materialIter->second.vkSet());
    }

    if (model->hasOverrideMaterials()) {

        auto* first { model->overrideMaterials().front() };
        if (first->_payload.diffuse->isLoaded() &&
            _STD ranges::find(__test_flag, static_cast<void*>(first)) == _STD ranges::end(__test_flag)
        ) {

            #pragma region Test Stream-Feedback
            #if TRUE
            {
                sptr<RevVirtualMarkerTexture> markerTexture {
                    _STD static_pointer_cast<RevVirtualMarkerTexture, void>(
                        data.find("RevDepthStage::MarkerTexture"sv)->second
                    )
                };

                auto it = data.find("RevEarlySFNode::CsfmBuffer"sv);
                if (it != data.end()) {

                    sptr<Buffer> csfm {
                        _STD static_pointer_cast<Buffer, void>(it->second)
                    };

                    /**/
                    const auto mttEntry { data.at("RevEarlySFNode::SfMtt"sv) };
                    auto& mtt { *_STD static_pointer_cast<render::RevSfMtt, void>(mttEntry) };
                    /**/

                    csfm->mapAligned();
                    auto* mapped { csfm->memory->mapping };
                    auto* casted { static_cast<ptr<u32>>(mapped) };

                    /**
                     * 1x1      ::  13.0/0 -> [0..1)
                     * 2x2      ::  12.0/1 -> [1..2)
                     * 4x4      ::  11.0/2 -> [2..3)
                     * 8x8      ::  10.0/3 -> [3..4)
                     * 16x16    ::  9.0/4 -> [4..5)
                     * 32x32    ::  8.0/5 -> [5..6)
                     * 64x64    ::  7.0/6 -> [6..7)
                     * 128x128  ::  6.0/7 -> [7..8)
                     * 256x256  ::  5.0/8 -> [8..12)
                     * 512x512  ::  4.0/9 -> [12..28)
                     * 1k x 1k  ::  3.0/10 -> [28..92)
                     * 2k x 2k  ::  2.0/11 -> [92..348)
                     * 4k x 4k  ::  1.0/12 -> [348..1372)
                     * 8k x 8k  ::  0.0/13 -> [1372..5468)
                     */

                    /**/

                    #if FALSE
                    auto indices {
                        RevVirtualMarkerTexture::tileBitToIndex(
                            static_cast<const ptr<const u32>>(csfm->memory->mapping)
                        )
                    };

                    csfm->unmap();

                    auto* diff { first->_payload.diffuse };
                    auto& view { diff->_payload.view };

                    for (auto&& sfi : indices) {

                        auto [mip, offset] = markerTexture->tileFromIndex(sfi);

                        state->cacheCtrl.markAsUsed(diff, {
                            .layer = view->baseLayer(),
                            .mip = static_cast<u32>(mip),
                            .offset = _STD move(offset),
                            .extent = markerTexture->tileExtent(mip)
                        });
                    }
                    #endif

                    /**/
                    for (const auto& material : model->overrideMaterials()) {

                        /**
                         * Get the csfm data offset by forward querying the material's dynamic index
                         */
                        const auto dynIdx { mtt.forward.at(material) };
                        const auto offset { dynIdx * (2ui32 + 171ui32) };

                        /**/
                        if (offset >= csfm->memory->size) {
                            continue;
                        }
                        /**/

                        /**
                         *
                         */
                        const auto indices {
                            RevVirtualMarkerTexture::tileBitToIndex(
                                &static_cast<const ptr<const u32>>(csfm->memory->mapping)[offset]
                            )
                        };

                        csfm->unmap();

                        auto* diff { material->_payload.diffuse };
                        auto& view { diff->_payload.view };

                        auto* norm { material->_payload.normal };
                        auto& normView { norm->_payload.view };

                        auto* rough { material->_payload.roughness };
                        auto& roughView { rough->_payload.view };

                        auto* metal { material->_payload.metalness };
                        auto& metalView { metal->_payload.view };

                        auto* ao { material->_payload.ao };
                        auto& aoView { ao->_payload.view };

                        for (auto&& sfi : indices) {

                            auto [mip, offset] = markerTexture->tileFromIndex(sfi);

                            auto aksr {
                                AssocKey<cache::TextureSubResource>::from({
                                    .layer = view->baseLayer(),
                                    .mip = static_cast<u32>(mip),
                                    .offset = offset,
                                    .extent = markerTexture->tileExtent(mip)
                                })
                            };

                            assert(normView->baseLayer() == aksr.value.layer);
                            assert(roughView->baseLayer() == aksr.value.layer);
                            assert(metalView->baseLayer() == aksr.value.layer);
                            assert(aoView->baseLayer() == aksr.value.layer);

                            state->cacheCtrl.markAsUsed(diff, aksr);
                            state->cacheCtrl.markAsUsed(norm, aksr);
                            state->cacheCtrl.markAsUsed(rough, aksr);
                            state->cacheCtrl.markAsUsed(metal, aksr);
                            state->cacheCtrl.markAsUsed(ao, aksr);
                        }
                    }
                }
            }
            #endif
            #pragma endregion

            #pragma region Test Virtual Texture Streaming
            #if FALSE
            auto* diff { first->_payload.diffuse };
            auto& view { diff->_payload.view };

            for (auto mip { 6ui32 }; mip <= view->maxMipLevel(); ++mip) {
                // TODO:
                // Warning: This only works if view extent is pow of 2
                state->cacheCtrl.markAsUsed(diff, {
                    .layer = view->baseLayer(),
                    //.mip = view->minMipLevel(),
                    .mip = mip,
                    .offset = math::uivec3 {},
                    .extent = math::uivec3 {
                        view->width() >> mip,
                        view->height() >> mip,
                        //view->depth() >> mip
                        view->depth()
                    }
                });
            }

            for (u32 i { 5ui32 }; i > 2; --i) {
                const auto th = view->height() >> i;
                const auto tw = view->width() >> i;

                for (u32 d = 1; d < 2; ++d) {
                    for (u32 h = 128ui32; h <= th; h += 128ui32) {
                        for (u32 w = 128ui32; w <= tw; w += 128ui32) {
                            state->cacheCtrl.markAsUsed(diff, {
                                .layer = view->baseLayer(),
                                //.mip = view->minMipLevel(),
                                .mip = i,
                                .offset = math::uivec3 {
                                    w - 128ui32,
                                    h - 128ui32,
                                    d - 1ui32
                                },
                                .extent = math::uivec3 {
                                    128ui32,
                                    128ui32,
                                    1ui32
                                }
                            });
                        }
                    }
                }
            }
            #endif
            #pragma endregion

            const_cast<ptr<RevMainStaticNode>>(this)->__test_flag.push_back(first);
        }
    }

    /**
     *
     */
    cmd.bindVertexBuffer(0, res->_vertexData.buffer, 0);
    cmd.bindIndexBuffer(res->_indexData.buffer, 0);

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

    // TODO: -> Actual target method
    auto* const batch { model_->batch(state) };
    auto* const casted { static_cast<const ptr<StaticGeometryBatch>>(batch) };

    // TODO: Temporary storage
    if (casted->cdb != nullptr) {
        auto& dbg { casted->cdb->binding() };
        cmd.bindDescriptor(sbgIdx, dbg.vkSet());

    } else {
        /**
         * Acquire Descriptor for model data, push data and bind it
         */
        auto& cache { renderPass_->state()->bindingCache };
        auto dbg { cache.allocate(*sbg) };

        dbg.getById(2ui32).store(casted->instance);
        cmd.bindDescriptor(sbgIdx, dbg.vkSet());

        if (casted->cdb == nullptr) {
            casted->cdb = new CachedDiscreteBinding { _STD move(dbg) };
        } else {
            casted->cdb->operator=(_STD move(dbg));
        }
    }

    /**
     * Invoke Rendering Code
     */
    //cmd.drawIndexed(1, 0, ... / sizeof(u32), 0ui32, 0ui32);
    //cmd.drawIndexed(1, 0, 1140ui32, 0ui32, 0ui32);
    auto* asset { static_cast<const ptr<const assets::StaticGeometry>>(res->origin()) };
    cmd.drawIndexed(1, 0, asset->getIndexCount(), 0ui32, 0ui32);
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

    shader::PrototypeBinding material {
        shader::BindingType::eUniformBuffer,
        3ui32,
        shader::BindingUpdateInterval::eMaterialUpdate,
        "staticMainMaterial"
    };

    shader::PrototypeBinding albedo {
        shader::BindingType::eImageSampler,
        4ui32,
        shader::BindingUpdateInterval::eMaterialUpdate,
        "staticMainPassAlbedo"
    };

    shader::PrototypeBinding normal {
        shader::BindingType::eImageSampler,
        5ui32,
        shader::BindingUpdateInterval::eMaterialUpdate,
        "staticMainPassNormal"
    };

    shader::PrototypeBinding roughness {
        shader::BindingType::eImageSampler,
        6ui32,
        shader::BindingUpdateInterval::eMaterialUpdate,
        "staticMainPassRoughness"
    };

    shader::PrototypeBinding metalness {
        shader::BindingType::eImageSampler,
        7ui32,
        shader::BindingUpdateInterval::eMaterialUpdate,
        "staticMainPassMetalness"
    };

    shader::PrototypeBinding specular {
        shader::BindingType::eImageSampler,
        8ui32,
        shader::BindingUpdateInterval::eMaterialUpdate,
        "staticMainPassSpecular"
    };

    shader::PrototypeBinding alpha {
        shader::BindingType::eImageSampler,
        9ui32,
        shader::BindingUpdateInterval::eMaterialUpdate,
        "staticMainPassAlpha"
    };

    shader::PrototypeBinding ao {
        shader::BindingType::eImageSampler,
        10ui32,
        shader::BindingUpdateInterval::eMaterialUpdate,
        "staticMainPassAO"
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
    fragmentPrototype.add(material);
    fragmentPrototype.add(albedo);
    fragmentPrototype.add(normal);
    fragmentPrototype.add(roughness);
    fragmentPrototype.add(metalness);
    fragmentPrototype.add(specular);
    fragmentPrototype.add(alpha);
    fragmentPrototype.add(ao);

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

shader::DiscreteBindingGroup RevMainStaticNode::createMaterialDescriptor(const ptr<RenderPassState> state_) const {

    const auto poolIt { state_->data.find("RevMainStaticNode::DescriptorPools"sv) };
    sptr<Vector<vk::DescriptorPool>> pools {
        _STD static_pointer_cast<Vector<vk::DescriptorPool>, void>(poolIt->second)
    };

    const auto& poolReq { _requiredDescriptorPools[MATERIAL_DESCRIPTOR_INDEX] };
    auto pool { _device->vkDevice().createDescriptorPool(poolReq) };

    const auto& setReq { _requiredBindingGroups[MATERIAL_DESCRIPTOR_INDEX] };
    auto set { _device->vkDevice().allocateDescriptorSets({ pool, 1ui32, &setReq.vkSetLayout() })[0] };

    pools->push_back(pool);

    return _STD move(setReq.useDiscrete(set));
}
