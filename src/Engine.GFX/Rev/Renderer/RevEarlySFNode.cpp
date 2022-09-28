#include "RevEarlySFNode.hpp"

#include <Engine.GFX/VkComputePipeline.hpp>
#include <Engine.GFX/API/VkTranslate.hpp>
#include <Engine.GFX/Command/CommandBuffer.hpp>
#include <Engine.GFX/Command/CommandPool.hpp>
#include <Engine.GFX/Command/CommandQueue.hpp>
#include <Engine.GFX/Performance/RenderTokens.hpp>
#include <Engine.GFX/Renderer/HORenderPass.hpp>
#include <Engine.GFX/Renderer/RenderPassState.hpp>
#include <Engine.GFX/Renderer/RenderStagePass.hpp>
#include <Engine.GFX/Shader/Factory.hpp>
#include <Engine.GFX/Shader/ShaderStorage.hpp>

#include <Engine.GFX/Rev/Texture/RevVirtualMarkerTexture.hpp>

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

#ifdef _DEBUG
#include <cassert>
#endif

using namespace ember::engine::gfx::render;
using namespace ember::engine::gfx;
using namespace ember;

RevEarlySFNode::RevEarlySFNode() {}

void RevEarlySFNode::setup(cref<sptr<Device>> device_) {

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
     * Compute Pipeline
     */
    _pipeline = make_sptr<VkComputePipeline>(device_);
    assert(_pipeline);

    _pipeline->shaderSlot().name() = "earlySFPass";

    /**
     *
     */
    _pipeline->setup();
}

void RevEarlySFNode::destroy() {

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

bool RevEarlySFNode::allocate(const ptr<HORenderPass> renderPass_) {

    SCOPED_STOPWATCH

    const auto state { renderPass_->state() };

    /**
     * Allocate Command Buffer
     */
    auto cmd { _device->computeQueue()->pool()->make() };

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
     * Early immutable storage bindings
     */
    auto depthMarkerEntry { state->data.find("RevDepthStage::Framebuffer"sv) };
    assert(depthMarkerEntry != state->data.end());

    const sptr<::ember::engine::gfx::Framebuffer> markerTexture {
        _STD static_pointer_cast<::ember::engine::gfx::Framebuffer, void>(depthMarkerEntry->second)
    };

    dbgs[0].getById(shader::ShaderBinding::id_type { 1 }).storeAdv(
        *markerTexture->attachments()[1ui32].unwrapped().get(),
        vk::ImageLayout::eShaderReadOnlyOptimal,
        vk::SamplerAddressMode::eClampToBorder,
        vk::SamplerMipmapMode::eNearest,
        vk::Filter::eNearest
    );

    // Warning: Temporary Solution to test
    {
        constexpr auto TEST_MATERIAL_COUNT = 8;

        /**
         * Allocate MTT Buffer
         */
        Buffer mtt {
            nullptr,
            nullptr,
            _device->vkDevice(),
            MAX((sizeof(u16) * TEST_MATERIAL_COUNT), 128ui32),
            vk::BufferUsageFlagBits::eStorageBuffer
        };

        const vk::BufferCreateInfo bci {
            vk::BufferCreateFlags {},
            mtt.size,
            mtt.usageFlags,
            vk::SharingMode::eExclusive,
            0ui32,
            nullptr
        };

        mtt.buffer = _device->vkDevice().createBuffer(bci);
        assert(mtt.buffer);

        const auto result {
            memory::allocate(&state->alloc, _device, mtt.buffer, MemoryProperty::eHostVisible, mtt.memory)
        };// TODO: Handle failed allocation
        mtt.bind();

        /**
         * Default insert data
         */
        mtt.mapAligned();

        for (auto fwd { 0ui8 }; fwd < (TEST_MATERIAL_COUNT / 2); ++fwd) {
            static_cast<ptr<u32>>(mtt.memory->mapping)[fwd] |= static_cast<u16>(fwd) * 2;
            static_cast<ptr<u32>>(mtt.memory->mapping)[fwd] |= static_cast<u32>(static_cast<u16>(fwd) * 2 + 1) << 16;
        }

        mtt.flushAligned();
        mtt.unmap();

        /**
         * Early bind buffer to descriptor
         */
        dbgs[0].getById(shader::ShaderBinding::id_type { 2 }).store(mtt);

        /**
         * Store to state
         */
        state->data.insert_or_assign("RevEarlySFNode::MttBuffer"sv, _STD make_shared<decltype(mtt)>(_STD move(mtt)));
    }

    // Warning: Temporary Solution to test
    {
        constexpr auto TEST_MATERIAL_COUNT = 8;

        constexpr auto distinct_count = 5468;
        constexpr auto distinct_size = ((distinct_count / 8) + ((distinct_count % 8) ? 1 : 0));
        constexpr auto distinct_aligned_size = (distinct_size / sizeof(u32)) * sizeof(u32) + (
            (distinct_size % sizeof(u32)) ? 4 : 0);

        /**
         * Allocate CSFM Buffer
         */
        Buffer csfm {
            nullptr,
            nullptr,
            _device->vkDevice(),
            distinct_aligned_size * TEST_MATERIAL_COUNT,
            vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst
        };

        const vk::BufferCreateInfo bci {
            vk::BufferCreateFlags {},
            csfm.size,
            csfm.usageFlags,
            vk::SharingMode::eExclusive,
            0ui32,
            nullptr
        };

        csfm.buffer = _device->vkDevice().createBuffer(bci);
        assert(csfm.buffer);

        const auto result {
            memory::allocate(&state->alloc, _device, csfm.buffer,
                MemoryProperty::eHostVisible | MemoryProperty::eHostCoherent, csfm.memory)
        };// TODO: Handle failed allocation
        csfm.bind();

        /**
         * Default insert data
         */
        csfm.mapAligned();
        //csfm.flushAligned();
        csfm.unmap();

        /**
         * Early bind buffer to descriptor
         */
        dbgs[0].getById(shader::ShaderBinding::id_type { 3 }).store(csfm);

        /**
         * Store to state
         */
        state->data.insert_or_assign("RevEarlySFNode::CsfmBuffer"sv, _STD make_shared<decltype(csfm)>(_STD move(csfm)));
    }

    /**
     * Store State
     */
    state->data.insert_or_assign("RevEarlySFNode::CommandBuffer"sv,
        _STD make_shared<decltype(cmd)>(_STD move(cmd)));
    state->data.insert_or_assign("RevEarlySFNode::DiscreteBindingGroups"sv,
        _STD make_shared<decltype(dbgs)>(_STD move(dbgs)));
    state->data.insert_or_assign("RevEarlySFNode::DescriptorPools"sv,
        _STD make_shared<decltype(pools)>(_STD move(pools)));

    /**
     * Post Process
     */
    postProcessAllocated(renderPass_);

    return true;
}

bool RevEarlySFNode::free(const ptr<HORenderPass> renderPass_) {

    SCOPED_STOPWATCH

    const auto state { renderPass_->state() };

    /**
     * Free Descriptors
     */
    auto it { state->data.find("RevEarlySFNode::DiscreteBindingGroups"sv) };
    if (it != state->data.end()) {

        sptr<Vector<shader::DiscreteBindingGroup>> dbgs {
            _STD static_pointer_cast<Vector<shader::DiscreteBindingGroup>, void>(it->second)
        };

        const auto poolIt { state->data.find("RevEarlySFNode::DescriptorPools"sv) };
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
    it = state->data.find("RevEarlySFNode::MttBuffer"sv);
    if (it != state->data.end()) {

        sptr<Buffer> mtt {
            _STD static_pointer_cast<Buffer, void>(it->second)
        };

        /**
         *
         */
        mtt->destroy();

        /**
         *
         */
        state->data.erase(it);
    }

    it = state->data.find("RevEarlySFNode::CsfmBuffer"sv);
    if (it != state->data.end()) {

        sptr<Buffer> csfm {
            _STD static_pointer_cast<Buffer, void>(it->second)
        };

        /**
         *
         */
        csfm->destroy();

        /**
         *
         */
        state->data.erase(it);
    }

    /**
     * Free Command Buffers
     */
    it = renderPass_->state()->data.find("RevEarlySFNode::CommandBuffer"sv);
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

Vector<RenderDataToken> RevEarlySFNode::requiredToken() noexcept {
    return {};
}

Vector<RenderDataToken> RevEarlySFNode::optionalToken() noexcept {
    return {};
}

void RevEarlySFNode::before(const non_owning_rptr<HORenderPass> renderPass_,
    const non_owning_rptr<RenderStagePass> stagePass_) const {

    SCOPED_STOPWATCH

    const auto& data { renderPass_->state()->data };

    /**
     * Get Command Buffer
     */
    const auto cmdEntry { data.at("RevEarlySFNode::CommandBuffer"sv) };
    auto& cmd { *_STD static_pointer_cast<CommandBuffer, void>(cmdEntry) };

    cmd.begin();

    /**
     *
     */
    cmd.bindPipeline(_pipeline.get());

    {
        auto it = data.find("RevEarlySFNode::CsfmBuffer"sv);
        if (it != data.end()) {

            sptr<Buffer> csfm {
                _STD static_pointer_cast<Buffer, void>(it->second)
            };

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

            // TODO: Clear previous render data
            cmd.vkCommandBuffer().fillBuffer(csfm->buffer, 0ui32, csfm->size, 0ui32);
        }
    }

    /**
     * Bind Shared Resources for the whole Frame
     */
    sptr<Vector<shader::DiscreteBindingGroup>> dbgs {
        _STD static_pointer_cast<Vector<shader::DiscreteBindingGroup>, void>(
            data.find("RevEarlySFNode::DiscreteBindingGroups"sv)->second
        )
    };

    for (u32 idx = 0; idx < dbgs->size(); ++idx) {

        const auto& grp { (*dbgs)[idx] };

        if (grp.super().interval() == shader::BindingUpdateInterval::ePerFrame) {
            cmd.bindDescriptor(idx, grp.vkSet());
        }
    }
}

void RevEarlySFNode::invoke(const non_owning_rptr<HORenderPass> renderPass_,
    const non_owning_rptr<RenderStagePass> stagePass_, const non_owning_rptr<SceneNodeModel> model_) const {

    SCOPED_STOPWATCH

    auto* state { renderPass_->state().get() };
    const auto& data { renderPass_->state()->data };

    /**
     * Get Command Buffer
     */
    const auto cmdEntry { data.at("RevEarlySFNode::CommandBuffer"sv) };
    auto& cmd { *_STD static_pointer_cast<CommandBuffer, void>(cmdEntry) };

    /**
     * Invoke Computing
     */
    constexpr auto grpX { 1ui64 };
    constexpr auto grpY { 1ui64 };
    constexpr auto grpZ { 1ui64 };

    cmd.vkCommandBuffer().dispatch(grpX, grpY, grpZ);
}

void RevEarlySFNode::after(const non_owning_rptr<HORenderPass> renderPass_,
    const non_owning_rptr<RenderStagePass> stagePass_) const {

    SCOPED_STOPWATCH

    const auto& data { renderPass_->state()->data };

    /**
     * Get Command Buffer
     */
    const auto cmdEntry { data.at("RevEarlySFNode::CommandBuffer"sv) };
    auto& cmd { *_STD static_pointer_cast<CommandBuffer, void>(cmdEntry) };

    /**
     * End Command Buffer
     */
    cmd.end();

    /**
     * Submit Command Buffer
     */
    stagePass_->batch().push(cmd);
}

void RevEarlySFNode::setupShader(cref<sptr<Device>> device_) {

    /**
     *
     */
    shader::Factory shaderFactory { device_ };
    ref<ShaderStorage> shaderStorage { ShaderStorage::get() };

    /**
     * Prepare Prototype Bindings
     */
    shader::PrototypeBinding cmt {
        // Compact Material(Id) Tile(Id)
        shader::BindingType::eImageSampler,
        1ui32,
        shader::BindingUpdateInterval::ePerFrame,
        "earlySFPassCmt"
    };

    shader::PrototypeBinding mtt {
        // Material Translate Table
        shader::BindingType::eStorageBuffer,
        2ui32,
        shader::BindingUpdateInterval::ePerFrame,
        "earlySFPassMtt"
    };

    shader::PrototypeBinding csfm {
        // Compact Stream Feedback Masking
        shader::BindingType::eStorageBuffer,
        3ui32,
        shader::BindingUpdateInterval::ePerFrame,
        "earlySFPassCfsm"
    };

    /**
     * Prepare Prototype Shader
     */
    shader::Prototype prototype {
        shader::ShaderType::eCompute,
        "earlySFPass"
    };

    auto computeShaderCode { read_shader_file("resources/shader/earlypass_sf.comp.spv") };
    prototype.storeCode(computeShaderCode.data(), computeShaderCode.size());
    prototype.add(cmt);
    prototype.add(mtt);
    prototype.add(csfm);

    /**
     * Build Shader and Bindings
     */
    auto factoryResult {
        shaderFactory.build({
            prototype
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
                _STD find_if(sizes.begin(), sizes.end(), [type= binding.type()](cref<vk::DescriptorPoolSize> entry_) {
                    return entry_.type == api::vkTranslateBindingType(type);
                })
            };

            if (it == sizes.end()) {
                sizes.push_back({ api::vkTranslateBindingType(binding.type()), 1 });
                continue;
            }

            ++(it->descriptorCount);
        }

        /**
         * Persist data structure
         */
        auto* sizeMem { new vk::DescriptorPoolSize[sizes.size()] };
        for (u32 i { 0 }; i < sizes.size(); ++i) {
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

void RevEarlySFNode::postProcessAllocated(const ptr<HORenderPass> renderPass_) {

    SCOPED_STOPWATCH

    const auto& data { renderPass_->state()->data };

    /**
     * Get Command Buffer
     */
    auto* const pool = _device->graphicsQueue()->pool();
    pool->lck().acquire();
    CommandBuffer cmd = pool->make();
    cmd.begin();

    /**
     * Transform / Pre-Fill
     */
    const auto it = data.find("RevEarlySFNode::CsfmBuffer"sv);
    if (it != data.end()) {

        const sptr<Buffer> csfm {
            _STD static_pointer_cast<Buffer, void>(it->second)
        };

        cmd.vkCommandBuffer().fillBuffer(csfm->buffer, 0ui32, csfm->size, 0ui32);
    }

    /**
     * Submiit
     */
    cmd.end();
    cmd.submitWait();
    cmd.release();

    pool->lck().release();
}
