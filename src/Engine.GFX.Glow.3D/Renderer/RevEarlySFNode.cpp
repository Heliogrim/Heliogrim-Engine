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

#include <Engine.GFX.Glow/Texture/RevVirtualMarkerTexture.hpp>

#include "State/RevSfMtt.hpp"

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

#ifdef _DEBUG
#include <cassert>
#endif

using namespace hg::engine::gfx::glow::render;
using namespace hg::engine::gfx::render;
using namespace hg::engine::gfx;
using namespace hg;

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

    const sptr<::hg::engine::gfx::Framebuffer> markerTexture {
        _STD static_pointer_cast<::hg::engine::gfx::Framebuffer, void>(depthMarkerEntry->second)
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
        /**
         * Store to state
         */
        auto sfMtt { _STD make_shared<RevSfMtt>() };
        state->data.insert_or_assign("RevEarlySFNode::SfMtt"sv, sfMtt);

        [[maybe_unused]] auto builtMtt = rebuildMttBuffer(sfMtt, state);
        [[maybe_unused]] auto builtCsfm = rebuildCsfmBuffer(sfMtt, state);

        //
        const auto mttEntry { state->data.at("RevEarlySFNode::MttBuffer"sv) };
        auto& mtt { *_STD static_pointer_cast<Buffer, void>(mttEntry) };

        dbgs[0].getById(shader::ShaderBinding::id_type { 2 }).store(mtt);

        //
        const auto csfmEntry { state->data.at("RevEarlySFNode::CsfmBuffer"sv) };
        auto& csfm { *_STD static_pointer_cast<Buffer, void>(mttEntry) };

        dbgs[0].getById(shader::ShaderBinding::id_type { 3 }).store(csfm);
    }

    /**
     * Store State
     */
    state->data.insert_or_assign(
        "RevEarlySFNode::CommandBuffer"sv,
        _STD make_shared<decltype(cmd)>(_STD move(cmd))
    );
    state->data.insert_or_assign(
        "RevEarlySFNode::DiscreteBindingGroups"sv,
        _STD make_shared<decltype(dbgs)>(_STD move(dbgs))
    );
    state->data.insert_or_assign(
        "RevEarlySFNode::DescriptorPools"sv,
        _STD make_shared<decltype(pools)>(_STD move(pools))
    );

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
     * Free Tables
     */
    it = state->data.find("RevEarlySFNode::SfMtt"sv);
    if (it != state->data.end()) {

        auto& mtt { *_STD static_pointer_cast<RevSfMtt, void>(it->second) };

        /**
         *
         */
        state->data.erase(it);
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

void RevEarlySFNode::before(
    const non_owning_rptr<HORenderPass> renderPass_,
    const non_owning_rptr<RenderStagePass> stagePass_
) const {

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

    /**/
    #pragma region Experimental Rebuilding and Binding
    {
        /**
         * Checking for requirement to rebuild buffers storing stream feedback
         * -> This should actualy be done by async enqueue job requiring a certain synchronization block between gpu and cpu
         * -> Memory should actually work just fine, because we can use `UpdateAfterBind` descriptors which "just" require
         *  an external additional sychronization
         */

        const auto sfMttEntry { data.at("RevEarlySFNode::SfMtt"sv) };
        auto sfMtt { _STD static_pointer_cast<RevSfMtt, void>(sfMttEntry) };

        if (rebuildMttBuffer(sfMtt, renderPass_->state())) {

            const auto mttEntry { renderPass_->state()->data.at("RevEarlySFNode::MttBuffer"sv) };
            auto& mtt { *_STD static_pointer_cast<Buffer, void>(mttEntry) };

            (*dbgs)[0].getById(shader::ShaderBinding::id_type { 2 }).store(mtt);
        }

        if (rebuildCsfmBuffer(sfMtt, renderPass_->state())) {

            const auto csfmEntry { renderPass_->state()->data.at("RevEarlySFNode::CsfmBuffer"sv) };
            auto& csfm { *_STD static_pointer_cast<Buffer, void>(csfmEntry) };

            (*dbgs)[0].getById(shader::ShaderBinding::id_type { 3 }).store(csfm);
        }
    }

    {
        const auto mttEntry { data.at("RevEarlySFNode::SfMtt"sv) };
        auto& mtt { *_STD static_pointer_cast<RevSfMtt, void>(mttEntry) };

        assert(mtt.mti <= 8ui16);

        const auto mttBufferEntry { data.at("RevEarlySFNode::MttBuffer"sv) };
        auto& mttBuffer { *_STD static_pointer_cast<Buffer, void>(mttBufferEntry) };

        //mttBuffer.write<u32>();
    }

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
            //cmd.vkCommandBuffer().fillBuffer(csfm->buffer, 0ui32, csfm->size, 0ui32);
        }
    }
    #pragma endregion
}

void RevEarlySFNode::invoke(
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
    const auto cmdEntry { data.at("RevEarlySFNode::CommandBuffer"sv) };
    auto& cmd { *_STD static_pointer_cast<CommandBuffer, void>(cmdEntry) };

    /**
     * Invoke Computing
     */
    constexpr auto workGroupSize { 256ui32 };

    const auto extent { renderPass_->target()->extent() };
    const auto payload { extent.x * extent.y * extent.z };

    const auto grpX { (payload / workGroupSize) + 1ui32 };
    constexpr auto grpY { 1ui32 };
    constexpr auto grpZ { 1ui32 };

    cmd.vkCommandBuffer().dispatch(grpX, grpY, grpZ);
}

void RevEarlySFNode::after(
    const non_owning_rptr<HORenderPass> renderPass_,
    const non_owning_rptr<RenderStagePass> stagePass_
) const {

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
        shaderFactory.build(
            {
                prototype
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
                    [type= binding.type()](cref<vk::DescriptorPoolSize> entry_) {
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

bool RevEarlySFNode::rebuildMttBuffer(cref<sptr<RevSfMtt>> sfMtt_, cref<sptr<RenderPassState>> state_) const {

    const auto mappedCount { MAX(sfMtt_->backward.size(), 1) };
    const auto mappedSize { mappedCount * sizeof(u16) };
    const auto alignedSize { (mappedSize % 128ui32 == 0) ? mappedSize : ((mappedSize / 128ui32) + 1ui32) * 128ui32 };

    //
    const auto prevEntry { state_->data.find("RevEarlySFNode::MttBuffer"sv) };
    if (prevEntry != state_->data.end()) {
        auto prev { _STD static_pointer_cast<Buffer, void>(prevEntry->second) };

        /**
         * Check whether we need to rebuild or just reuse the previous buffer
         */
        if (prev->size >= alignedSize) {
            return false;
        }

        prev->destroy();
        prev.reset();
    }

    /**
     * Allocate MTT Buffer
     */
    Buffer mtt {
        nullptr,
        nullptr,
        _device->vkDevice(),
        alignedSize,
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
        memory::allocate(&state_->alloc, _device, mtt.buffer, MemoryProperty::eHostVisible, mtt.memory)
    };// TODO: Handle failed allocation
    mtt.bind();

    /**
     * Default insert data
     */
    #if FALSE
    mtt.mapAligned();

    for (auto fwd { 0ui8 }; fwd < (TEST_MATERIAL_COUNT / 2); ++fwd) {
        static_cast<ptr<u32>>(mtt.memory->mapping)[fwd] |= static_cast<u16>(fwd) * 2;
        static_cast<ptr<u32>>(mtt.memory->mapping)[fwd] |= static_cast<u32>(static_cast<u16>(fwd) * 2 + 1) << 16;
    }

    mtt.flushAligned();
    mtt.unmap();
    #endif

    /**
     * Store to state
     */
    state_->data.insert_or_assign("RevEarlySFNode::MttBuffer"sv, _STD make_shared<decltype(mtt)>(_STD move(mtt)));

    return true;
}

bool RevEarlySFNode::rebuildCsfmBuffer(cref<sptr<RevSfMtt>> sfMtt_, cref<sptr<RenderPassState>> state_) const {

    /**
     * Map bit-mapped decisions first to byte and then to layout used size type (Bit -> Byte -> U32)
     */
    constexpr auto singleMarkCount { 5468 };
    constexpr auto singleMarkSize {
        (singleMarkCount % 8ui32 == 0) ? singleMarkCount / 8ui32 : ((singleMarkCount / 8ui32) + 1ui32)
    };
    constexpr auto markAlignedSize {
        (singleMarkSize % 4ui32 == 0) ? singleMarkSize : ((singleMarkSize / 4ui32) + 1ui32) * 4ui32
    };

    /**
     * Map bit-mapped header first octree structure ( suppresses byte alignment ) and then to layout used size type (Bit -> Octree -> Byte -> U32)
     */
    constexpr auto singleHeaderCount { 43 };
    constexpr auto singleHeaderSize {
        (singleHeaderCount % 8ui32 == 0) ? singleHeaderCount / 8ui32 : ((singleHeaderCount / 8ui32) + 1ui32)
    };
    constexpr auto headerAlignedSize {
        (singleHeaderSize % 4ui32 == 0) ? singleHeaderSize : ((singleHeaderSize / 4ui32) + 1ui32) * 4ui32
    };

    /**
     * Expand aligned marks with captured table entries and align to flushable size
     */
    const auto mappedCount { MAX(sfMtt_->backward.size() + 1, 1) };
    const auto mappedSize { mappedCount * (headerAlignedSize + markAlignedSize) };
    const auto alignedSize { (mappedSize % 128ui32 == 0) ? mappedSize : ((mappedSize / 128ui32) + 1ui32) * 128ui32 };

    //
    const auto prevEntry { state_->data.find("RevEarlySFNode::CsfmBuffer"sv) };
    if (prevEntry != state_->data.end()) {
        auto prev { _STD static_pointer_cast<Buffer, void>(prevEntry->second) };

        /**
         * Check whether we need to rebuild or just reuse the previous buffer
         */
        if (prev->size >= alignedSize) {
            return false;
        }

        prev->destroy();
        prev.reset();
    }

    /**
     * Allocate CSFM Buffer
     */
    Buffer csfm {
        nullptr,
        nullptr,
        _device->vkDevice(),
        alignedSize,
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
        memory::allocate(
            &state_->alloc,
            _device,
            csfm.buffer,
            MemoryProperty::eHostVisible | MemoryProperty::eHostCoherent,
            csfm.memory
        )
    };// TODO: Handle failed allocation
    csfm.bind();

    /**
     * Default insert data
     */
    csfm.mapAligned();
    //csfm.flushAligned();
    csfm.unmap();

    /**
     * Store to state
     */
    state_->data.insert_or_assign("RevEarlySFNode::CsfmBuffer"sv, _STD make_shared<decltype(csfm)>(_STD move(csfm)));

    return true;
}
